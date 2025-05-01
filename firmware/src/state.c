#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32l496xx.h>
#include <stdlib.h>


#include "../include/state.h"
#include "../include/clock.h"
#include "../include/eink.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/commands.h"
#include "../include/uart.h"
#include "../include/ff.h"

volatile enum STATES state = STATE_HOME_NAVIGATION;
volatile uint8_t render_pending = 0;
uint8_t fetch_decks = 1;

struct font curr_font;

extern volatile uint8_t curr_deck_selection;
struct deck main_deck = {0};
uint8_t num_decks = 0;
volatile uint8_t curr_menu_selection = 0;

volatile uint8_t shuffle = 0;
volatile uint8_t font_large = 1;
uint8_t left_handed = 1;

#define UART_DELIM    ((char)0xBC)
#define UART_EOF      ((char)0x00)
#define UART_BUF_SIZE (4096)

extern volatile uint8_t interrupts;

/* https://stackoverflow.com/questions/6127503/shuffle-array-in-c */
void shuffle_deck(struct deck* deck)
{
    srand(TIM2->CNT);

    if (deck->num_cards > 1) {
        size_t i;
        for (i = 0; i < deck->num_cards - 1; i++) {
          size_t j = i + rand() / (RAND_MAX / (deck->num_cards - i) + 1);
          struct flashcard t = deck->cards[j];
          deck->cards[j] = deck->cards[i];
          deck->cards[i] = t;
        }
    }
}

volatile struct evq q = {0};

uint8_t evq_push(enum EVENT e)
{
    __disable_irq();
    uint8_t next = (q.head + 1) % EVQ_SIZE;
    if (next == q.tail) {
        __enable_irq();
        return 0;    // full
    }
    q.buf[q.head] = e;
    q.head = next;
    __enable_irq();
    return 1;
}

enum EVENT evq_pop()
{
    __disable_irq();
    if (q.head == q.tail) {
        __enable_irq();
        return EVENT_NONE; // empty
    }
    enum EVENT e = q.buf[q.tail];
    q.tail = (q.tail + 1) % EVQ_SIZE;
    __enable_irq();
    return e;
}

char menu_names[3][MAX_NAME_SIZE] = {
    "STUDY",
    "DOWNLOAD",
    "SETTINGS"
};

void download_deck()
{
    eink_clear(0xFF);
    draw_header("DOWNLOADING DECK");
    eink_render_framebuffer();

    static char contents[UART_BUF_SIZE] = {0};
    char filename[MAX_NAME_SIZE] = {0};
    char c = 0;
    uint32_t i = 0;

    while (c != UART_DELIM) {
        c = uart_read_char();
        if (c == UART_DELIM) {
            filename[i] = 0;
            break;
        }
        filename[i] = c;
        i++;
    }


    /* 2. Read in file contents until EOF or something so that we know its over */
    i = 0;
    UINT wlen;
    while (c != UART_EOF) {
        c = uart_read_char();
        if (c == UART_EOF) break;
        contents[i] = c;
        i++;
    }

    /* 3. Create and open file on SD card with that name */
    FIL fil;
    FRESULT fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
    if (fr) {
        log_to_sd("CANT OPEN\n");
        log_to_sd(filename);
        log_to_sd("\n");
    }

    fr = f_write(&fil, contents, strlen(contents), &wlen);
    if (fr) {
        log_to_sd("CANT WRITE\n");
        log_to_sd(contents);
        log_to_sd("\n");
    }

    f_sync(&fil);
    f_close(&fil);

    state = STATE_DECK_NAVIGATION;
    render_pending = 1;
    fetch_decks = 1;
    delay_ms(10);
}

void draw_home_menu()
{
    char header[25];
    eink_clear(0xFF);
    /* +1 because it is 0 indexed */
    snprintf(header, 25, "SELECT AN OPTION: %d/%d", curr_menu_selection + 1, 3);
    draw_header(header);
    draw_menu(curr_menu_selection, menu_names, 3);
    eink_render_framebuffer();
    render_pending = 0;
}


char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {0};

void coalesce_events()
{
    enum EVENT event;
    while ((event = evq_pop()) != EVENT_NONE) {
        event_handler handler = event_handlers[state][event];
        handler();
        interrupts--;
    }
}

/* NOTE: This will always be running when there is no interrupt happening */
void state_machine()
{
    char buf[100];

    for (;;) {
        delay_ms(10);
        if (render_pending != 1) {
            if (interrupts == 0) continue;
        }

        coalesce_events();

        switch (state) {
        case STATE_DOWNLOAD:
            download_deck();

            break;

        case STATE_HOME_NAVIGATION:
            draw_home_menu();

            break;

        case STATE_DECK_NAVIGATION:
            /* Get deck names if we don't have them yet */
            if (fetch_decks) {
                curr_deck_selection = 0;
                num_decks = get_decks(deck_names);
                fetch_decks = 0;
                parse_metadata(num_decks, deck_names);
                delay_ms(10);
            }

            eink_clear(0xFF);

            char header[25];
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "SELECT A DECK: %d/%d", curr_deck_selection + 1, num_decks);
            draw_header(header);
            draw_main_menu(curr_deck_selection, deck_names, num_decks);
            eink_render_framebuffer();
            render_pending = 0;

            break;

        case STATE_DELETE_DECK_CONFIRM:

            eink_clear(0xFF);
            snprintf(header, 25, "CONFIRM DELETION");
            draw_header(header);

            snprintf(buf, 100, "Long press SELECT to delete %s", deck_names[curr_deck_selection]);
            draw_string(curr_font, 15, 50, buf, BLACK);
            snprintf(buf, 100, "Long press BACK to return to main menu");
            draw_string(curr_font, 15, 70, buf, BLACK);
            eink_render_framebuffer();
            render_pending = 0;

            break;

        case STATE_SETTINGS:

            eink_clear(0xFF);
            snprintf(header, 25, "SETTINGS");
            draw_header(header);
            if (font_large) {
                curr_font = xlarge_font;
            } else {
                curr_font = large_font;
            }

            uint8_t old_left = left_handed;
            snprintf(buf, 100, "Press SELECT to toggle shuffling");
            draw_string(curr_font, 15, 50, buf, BLACK);
            snprintf(buf, 100, "SHUFFLE: %s", shuffle ? "YES" : "NO");
            draw_string(curr_font, 15, 70, buf, BLACK);
            snprintf(buf, 100, "Press FORWARD to change font size");
            draw_string(curr_font, 15, 90, buf, BLACK);
            snprintf(buf, 100, "FONT SIZE: %s", font_large ? "LARGE" : "SMALL");
            draw_string(curr_font, 15, 110, buf, BLACK);
            snprintf(buf, 100, "Press BACKWARD to select dominant hand");
            draw_string(curr_font, 15, 130, buf, BLACK);
            snprintf(buf, 100, "DOMINANT HAND: %s", left_handed ? "LEFT-HANDED" : "RIGHT-HANDED");
            draw_string(curr_font, 15, 150, buf, BLACK);

            eink_init();
            eink_render_framebuffer();

            render_pending = 0;
            break;

        case STATE_SLEEPING:
            if (!render_pending) break;
            interrupts--;
            eink_clear(0xFF);
            draw_header("SLEEPING");
            draw_string(curr_font, 70, 50, "START STUDYING AGAIN SOON!", BLACK);
            draw_sleep_image(100, 75);
            eink_render_framebuffer();
            render_pending = 0;
            break;

        case STATE_FLASHCARD_NAVIGATION:
            if (get_deck_from_sd) {
                parseJSON_file(deck_names[curr_deck_selection], &main_deck);
                get_deck_from_sd = 0;
                if (shuffle) {
                    shuffle_deck(&main_deck);
                }
            }

            eink_clear(0xFF);
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "%s | %d/%d", deck_names[curr_deck_selection], curr_card_selection + 1, main_deck.num_cards);
            draw_header(header);
            draw_flashcard(main_deck.cards[curr_card_selection], f_b, BLACK);
            eink_render_framebuffer();
            render_pending = 0;

            break;
        }
    }
}
