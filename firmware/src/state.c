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
// volatile enum STATES state = STATE_SLEEPING;
volatile uint8_t render_pending = 0;
uint8_t fetch_decks = 1;

extern volatile uint8_t curr_deck_selection;
struct deck main_deck;
uint8_t num_decks = 0;
volatile uint8_t curr_menu_selection = 0;

extern volatile uint8_t press;
extern volatile uint8_t btn;

volatile uint8_t shuffle = 0;
volatile uint8_t learning_algo = 0;

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

char menu_names[3][MAX_NAME_SIZE] = {
    "STUDY",
    "DOWNLOAD",
    "SETTINGS"
};

char settings_names[2][MAX_NAME_SIZE] = {
    "SHUFFLE",
    "LEARNING ALGO",
};

char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {
    "ECE 20002",
    "ECE 404",
    "PHIL 322"
};

/* NOTE: This will always be running when there is no interrupt happening */
void state_machine()
{
    // char deck_names[MAX_DECKS][MAX_NAME_SIZE] = {0};
    char header[25];

    char buf[100];
    for (;;) {
        delay_ms(10);
        switch (state) {
        case STATE_DOWNLOAD:
            // __disable_irq();

            eink_clear(0xFF);
            draw_header("DOWNLOADING DECK");
            eink_render_framebuffer();

            char contents[UART_BUF_SIZE] = {0};
            char filename[MAX_NAME_SIZE] = {0};
            char c = 0;
            uint32_t i = 0;

            while (c != UART_DELIM) {
                // if (btn == BACKWARD && press == LONG_PRESS) {
                //     break;
                // }
                c = uart_read_char();
                if (c == UART_DELIM) {
                    filename[i] = 0;
                    break;
                }
                filename[i++] = c;
            }

            // if (btn == BACKWARD && press == LONG_PRESS) {
            //     state = STATE_HOME_NAVIGATION;
            //     press = NO_PRESS;
            //     break;
            // }

            /* 2. Read in file contents until EOF or something so that we know its over */
            i = 0;
            UINT wlen;
            while (c != UART_EOF) {
                c = uart_read_char();
                if (c == UART_EOF) break;
                contents[i++] = c;
            }

            /* 3. Create and open file on SD card with that name */
            // FIL fil;
            // FRESULT fr = f_open(&fil, filename, FA_WRITE | FA_CREATE_NEW);
            // if (fr) {
                // log_to_sd("CANT OPEN\n");
                // log_to_sd(filename);
                // log_to_sd("\n");
            // }

            // fr = f_write(&fil, contents, strlen(contents), &wlen);
            // if (fr) {
                // log_to_sd("CANT WRITE\n");
                // log_to_sd(contents);
                // log_to_sd("\n");
            // }

            // f_sync(&fil);
            // f_close(&fil);

            eink_clear(0xFF);
            draw_centered_string_wrapped(large_font, contents, BLACK);
            eink_render_framebuffer();
            // __enable_irq();

            // state = STATE_MENU_NAVIGATION;
            // render_pending = 1;
            // fetch_decks = 1;

            break;

        case STATE_HOME_NAVIGATION:
            if (!render_pending) break;

            if (btn == SELECT) {
                if (press == SHORT_PRESS) {
                    if (curr_menu_selection == 0) {
                        state = STATE_MENU_NAVIGATION;
                    } else if (curr_menu_selection == 1) {
                        state = STATE_DOWNLOAD;
                    } else if (curr_menu_selection == 2) {
                        state = STATE_SETTINGS;
                    }
                    press = NO_PRESS;
                    break;
                } else if (press == LONG_PRESS) {
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == BACKWARD) {
                if (press == SHORT_PRESS) {
                    if (curr_menu_selection > 0) {
                        curr_menu_selection--;
                    }
                } else if (press == LONG_PRESS) {
                    state = STATE_DOWNLOAD;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == FORWARD) {
                if (press == SHORT_PRESS) {
                    if (curr_menu_selection + 1 < 3) {
                        curr_menu_selection++;
                    }
                } else if (press == LONG_PRESS) {
                    state = STATE_SETTINGS;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            }

            eink_clear(0xFF);
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "SELECT AN OPTION: %d/%d", curr_menu_selection + 1, 3);
            draw_header(header);
            draw_menu(curr_menu_selection, menu_names, 3);
            eink_render_framebuffer();
            render_pending = 0;
            break;

        case STATE_MENU_NAVIGATION:
            /* Get deck names if we don't have them yet */
            if (fetch_decks) {
                curr_deck_selection = 0;
                // num_decks = get_decks(deck_names);
                fetch_decks = 0;
            }

            if (!render_pending) break;

            if (btn == SELECT) {
                if (press == SHORT_PRESS) {
                    state = STATE_FLASHCARD_NAVIGATION;
                    get_deck_from_sd = 1;
                    press = NO_PRESS;
                    break;
                } else if (press == LONG_PRESS) {
                    /* Delete highlighted deck */
                    /* TODO: Add confirmation */
                    state = STATE_DELETE_DECK_CONFIRM;
                    // render_pending = 1;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == BACKWARD) {
                if (press == SHORT_PRESS) {
                    if (curr_deck_selection > 0) {
                        curr_deck_selection--;
                    }
                } else if (press == LONG_PRESS) {
                    state = STATE_HOME_NAVIGATION;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == FORWARD) {
                if (press == SHORT_PRESS) {
                    if (curr_deck_selection + 1 < num_decks) {
                        curr_deck_selection++;
                    }
                } else if (press == LONG_PRESS) {
                    state = STATE_SETTINGS;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            }

            eink_clear(0xFF);
            /* +1 because it is 0 indexed */
            snprintf(header, 25, "SELECT A DECK: %d/%d", curr_deck_selection + 1, num_decks);
            draw_header(header);
            draw_main_menu(curr_deck_selection, deck_names, num_decks);
            eink_render_framebuffer();
            render_pending = 0;

            break;

        case STATE_DELETE_DECK_CONFIRM:
            if (!render_pending) break;

            if (btn == SELECT) {
                if (press == SHORT_PRESS) {
                    break;
                } else if (press == LONG_PRESS) {
                    /* Delete highlighted deck */
                    // delete_file(deck_names[curr_deck_selection]);
                    fetch_decks = 1;
                    // render_pending = 1;
                    state = STATE_MENU_NAVIGATION;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == BACKWARD) {
                if (press == SHORT_PRESS) {
                    press = NO_PRESS;
                    break;
                } else if (press == LONG_PRESS) {
                    // render_pending = 1;
                    state = STATE_MENU_NAVIGATION;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == FORWARD) {
                if (press == SHORT_PRESS) {
                } else if (press == LONG_PRESS) {
                }
                press = NO_PRESS;
            }

            eink_clear(0xFF);
            snprintf(header, 25, "CONFIRM DELETION");
            draw_header(header);

            snprintf(buf, 100, "Long press SELECT to delete %s", deck_names[curr_deck_selection]);
            draw_string(large_font, 15, 50, buf, BLACK);
            snprintf(buf, 100, "Long press BACK to return to main menu");
            draw_string(large_font, 15, 70, buf, BLACK);
            eink_render_framebuffer();
            render_pending = 0;

            break;

        case STATE_SETTINGS:
            if (!render_pending) break;

            if (btn == SELECT) {
                if (press == SHORT_PRESS) {
                    shuffle = !shuffle;
                } else if (press == LONG_PRESS) {
                }
                press = NO_PRESS;
            } else if (btn == BACKWARD) {
                if (press == SHORT_PRESS) {
                    press = NO_PRESS;
                    break;
                } else if (press == LONG_PRESS) {
                    state = STATE_HOME_NAVIGATION;
                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == FORWARD) {
                if (press == SHORT_PRESS) {
                    learning_algo = !learning_algo;
                } else if (press == LONG_PRESS) {
                }
                press = NO_PRESS;
            }
            eink_clear(0xFF);
            snprintf(header, 25, "SETTINGS");
            draw_header(header);

            snprintf(buf, 100, "Press SELECT to toggle shuffling");
            draw_string(large_font, 15, 50, buf, BLACK);
            snprintf(buf, 100, "SHUFFLE: %s", shuffle ? "YES" : "NO");
            draw_string(large_font, 15, 70, buf, BLACK);
            snprintf(buf, 100, "Press FORWARD to toggle learning algo");
            draw_string(large_font, 15, 90, buf, BLACK);
            snprintf(buf, 100, "LEARNING ALGORITHM: %s", learning_algo ? "YES" : "NO");
            draw_string(large_font, 15, 110, buf, BLACK);
            // draw_menu(curr_menu_selection, settings_names, 2);
            eink_render_framebuffer();
            render_pending = 0;
            break;

        case STATE_SLEEPING:
            if (!render_pending) break;
            eink_clear(0xFF);
            draw_header("SLEEPING");
            draw_string(large_font, 100, 50, "START STUDYING AGAIN SOON!", BLACK);
            draw_sleep_image(100, 75);
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
                if (shuffle) {
                    shuffle_deck(&main_deck);
                }
            }
            if (!render_pending) break;

            if (btn == SELECT) {
                if (press == SHORT_PRESS) {
                    /* Flip */
                    f_b = !f_b;
                } else if (press == LONG_PRESS) {
                }
                press = NO_PRESS;
            } else if (btn == BACKWARD) {
                if (press == SHORT_PRESS) {
                    if (curr_card_selection > 0) {
                        curr_card_selection--;
                    }

                    /* Always start on the front */
                    if (f_b == BACK) f_b = FRONT;
                } else if (press == LONG_PRESS) {
                    state = STATE_MENU_NAVIGATION;

                    /* Don't save previous state when going back in the menu */
                    curr_deck_selection = 0;
                    curr_card_selection = 0;
                    f_b = FRONT;

                    press = NO_PRESS;
                    break;
                }
                press = NO_PRESS;
            } else if (btn == FORWARD) {
                if (press == SHORT_PRESS) {
                    if (curr_card_selection + 1 < main_deck.num_cards) {
                        curr_card_selection++;
                    }

                    /* Always start on the front */
                    if (f_b == BACK) f_b = FRONT;
                } else if (press == LONG_PRESS) {
                }
                press = NO_PRESS;
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
        // __WFI();
    }
}
