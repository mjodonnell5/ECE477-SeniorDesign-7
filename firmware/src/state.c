#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32l432xx.h>

#include "../include/state.h"
#include "../include/clock.h"
#include "../include/eink.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/commands.h"
#include "../include/uart.h"
#include "../include/ff.h"

// volatile enum STATES state = STATE_MENU_NAVIGATION;
volatile enum STATES state = STATE_DOWNLOAD;
volatile uint8_t render = 0;

extern volatile uint8_t curr_deck_selection;

#define UART_DELIM    ((char)0xBC)
#define UART_EOF      ((char)0x01)
#define UART_BUF_SIZE (16384)

/* NOTE: This will always be running when there is no interrupt happening */
void state_machine()
{
    uint8_t num_decks = 0;
    struct deck main_deck;
    char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {0};

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

            // eink_clear(0xFF);
            // draw_header("WHAT WE GOT");
            // draw_string(0, 20, filename, BLACK);
            // draw_centered_string_wrapped(buf, BLACK);
            // eink_render_framebuffer();

            __enable_irq();

            state = STATE_MENU_NAVIGATION;
            render = 1;

            break;

        case STATE_MENU_NAVIGATION:
            /* Get deck names if we don't have them yet */
            if (deck_names[0][0] == 0) {
                curr_deck_selection = 0;
                num_decks = get_decks(deck_names);
            }

            if (!render) break;

            eink_clear(0xFF);
            char header[25];
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "SELECT A DECK: %d/%d", curr_deck_selection + 1, num_decks);
            draw_header(header);
            draw_main_menu(curr_deck_selection, deck_names, num_decks);
            eink_render_framebuffer();
            render = 0;

            break;

        case STATE_FLASHCARD_NAVIGATION:
            if (get_deck_from_sd) {
                parseJSON_file(deck_names[curr_deck_selection], &main_deck);
                get_deck_from_sd = 0;
            }
            if (!render) break;

            eink_clear(0xFF);
            draw_header(main_deck.name);
            draw_flashcard(main_deck.cards[curr_card_selection], f_b, BLACK);
            eink_render_framebuffer();
            render = 0;

            break;
        }
        // __WFI();
    }
}
