#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stm32l432xx.h>

#include "../include/state.h"
#include "../include/clock.h"
#include "../include/eink.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/commands.h"
#include "../include/uart.h"
#include "../include/ff.h"

volatile enum STATES state = STATE_MENU_NAVIGATION;
// volatile enum STATES state = STATE_DOWNLOAD;
volatile uint8_t render_pending = 0;
uint8_t fetch_decks = 1;

extern volatile uint8_t curr_deck_selection;
struct deck main_deck;
uint8_t num_decks = 0;

#define UART_DELIM    ((char)0xBC)
#define UART_EOF      ((char)0x00)
#define UART_BUF_SIZE (16384)

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

/* NOTE: This will always be running when there is no interrupt happening */
void state_machine()
{
    char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {0};
    char header[25];

    for (;;) {
        delay_ms(10);
        switch (state) {
        case STATE_DOWNLOAD:
            __disable_irq();

            eink_clear(0xFF);
            draw_header("DOWNLOADING DECK");
            eink_render_framebuffer();

            char contents[UART_BUF_SIZE] = {0};
            char filename[MAX_NAME_SIZE] = {0};
            char c = 0;
            uint32_t i = 0;

            while (c != UART_DELIM) {
                c = uart_read_char();
                if (c == UART_DELIM) {
                    filename[i] = 0;
                    break;
                }
                filename[i++] = c;
            }

            /* 2. Read in file contents until EOF or something so that we know its over */
            i = 0;
            UINT wlen;
            while (c != UART_EOF) {
                c = uart_read_char();
                if (c == UART_EOF) break;
                contents[i++] = c;
            }

            /* 3. Create and open file on SD card with that name */
            FIL fil;
            FRESULT fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND);
            if (fr) {
                // log_to_sd("CANT OPEN\n");
                // log_to_sd(filename);
                // log_to_sd("\n");
            }

            fr = f_write(&fil, contents, strlen(contents), &wlen);
            if (fr) {
                // log_to_sd("CANT WRITE\n");
                // log_to_sd(contents);
                // log_to_sd("\n");
            }

            f_sync(&fil);
            f_close(&fil);

            __enable_irq();

            state = STATE_MENU_NAVIGATION;
            render_pending = 1;
            fetch_decks = 1;

            break;

        case STATE_MENU_NAVIGATION:
            /* Get deck names if we don't have them yet */
            if (fetch_decks) {
                curr_deck_selection = 0;
                num_decks = get_decks(deck_names);
                fetch_decks = 0;
            }

            if (!render_pending) break;

            eink_clear(0xFF);
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "SELECT A DECK: %d/%d", curr_deck_selection + 1, num_decks);
            draw_header(header);
            draw_main_menu(curr_deck_selection, deck_names, num_decks);
            eink_render_framebuffer();
            render_pending = 0;

            break;

        case STATE_DECK_HOME_PAGE:
        //     if (!render_pending) break;
        //     eink_clear(0xFF);
        //     snprintf(header, 25, "%s home", deck_names[curr_deck_selection]);
        //     draw_header(header);
        //     eink_render_framebuffer();
        //     render_pending = 0;
            break;

        case STATE_FLASHCARD_NAVIGATION:
            if (get_deck_from_sd) {
                parseJSON_file(deck_names[curr_deck_selection], &main_deck);
                get_deck_from_sd = 0;
                // shuffle_deck(&main_deck);
            }
            if (!render_pending) break;

            eink_clear(0xFF);
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "%s | %d/%d", deck_names[curr_deck_selection], curr_card_selection + 1, main_deck.num_cards);
            draw_header(header);
            draw_flashcard(main_deck.cards[curr_card_selection], f_b, BLACK);
            eink_render_framebuffer();
            render_pending = 0;

            break;
        }
        // __WFI();
    }
}
