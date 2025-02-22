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

volatile enum STATES state = STATE_MENU_NAVIGATION;
volatile uint8_t render = 0;

char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {0};
struct deck main_deck;
uint8_t num_decks = 0;

/* This will always be running when there is no interrupt happening */
void state_machine()
{
    uint8_t curr_page = 0;
    for (;;) {
        delay_ms(10);
        switch (state) {
        case STATE_DOWNLOAD:
            __disable_irq();
            /* 1. Receive deck name */
            /* 2. Create and open file on SD card with that name */
            /* 3. Read in file contents until EOF or something so that we know its over */
            /* 4. Write each char/chunk as we get it (probably wanna read into a
             * buffer and write that buffer to the file occasionally) */
            /* 5. Stop reading from UART once we get the EOF, close file */
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
            // char buf[MAX_BACK_SIZE]; /* Back is larger than front */
            draw_flashcard(main_deck.cards[curr_card_selection], f_b, BLACK);
            // if (f_b == FRONT) {
            //     snprintf(buf, MAX_FRONT_SIZE, "%s", main_deck.cards[curr_card_selection].front);
            // }
            // else if (f_b == BACK) {
            //     snprintf(buf, MAX_BACK_SIZE, "%s", main_deck.cards[curr_card_selection].back);
            // }
            // draw_string_wrapped(0, 20, buf, BLACK);
            eink_render_framebuffer();
            render = 0;

            break;
        }
        __WFI();
    }
}
