#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
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
volatile uint8_t render = 0;

char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {0};
struct deck main_deck;
uint8_t num_decks = 0;

#define UART_DELIM    (char)0xBC
#define UART_EOF      (char)0x00
#define UART_BUF_SIZE 1024
/* This will always be running when there is no interrupt happening */
void state_machine()
{
    uint8_t curr_page = 0;
    for (;;) {
        delay_ms(10);
        switch (state) {
        case STATE_DOWNLOAD:
            __disable_irq();

            draw_header("DOWNLOADING DECK");
            eink_render_framebuffer();

            char filename[MAX_NAME_SIZE];
            char c = 0;
            uint32_t i = 0;
            /* 1. Receive deck name */
            while (c != UART_DELIM) {
                c = uart_read_char();
                if (c == UART_DELIM) break;
                filename[i++] = c;
            }

            /* 2. Create and open file on SD card with that name */
            FIL fil;
            FRESULT fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
            if (fr) {
                /* uh oh! */
            }

            /* 3. Read in file contents until EOF or something so that we know its over */
            char buf[UART_BUF_SIZE];
            c = 0;
            i = 0;
            UINT wlen;
            while (c != UART_EOF) {
                c = uart_read_char();
                if (c == UART_EOF) break;
                buf[i++] = c;

                if (i >= UART_BUF_SIZE) {
                    f_write(&fil, buf, UART_BUF_SIZE, &wlen);
                }
            }

            /* Leftover */
            if (i > 0) {
                f_write(&fil, buf, UART_BUF_SIZE, &wlen);
            }
            __enable_irq();

            break;

        case STATE_MENU_NAVIGATION:
            /* Get deck names */
            if (deck_names[0][0] == 0) {
                num_decks = get_decks(deck_names);
            }

            if (!render) break;

            curr_page = curr_deck_selection / MAX_DECKS_PER_PAGE;

            eink_clear(0xFF);
            draw_main_menu(curr_deck_selection, deck_names, num_decks, curr_page);
            eink_render_framebuffer();
            render = 0;

            break;

        case STATE_DECK_SELECTION:

        case STATE_FLASHCARD_NAVIGATION:
            if (get_deck_from_sd) {
                /* TODO: Call function that returns deck */
                parseJSON_file(deck_names[curr_deck_selection], &main_deck);
                get_deck_from_sd = 0;
            }
            if (!render) break;

            eink_clear(0xFF);
            draw_header(deck_names[curr_deck_selection]);
            draw_flashcard(main_deck.cards[curr_card_selection], f_b, BLACK);
            eink_render_framebuffer();
            render = 0;

            break;
        }
        __WFI();
    }
}
