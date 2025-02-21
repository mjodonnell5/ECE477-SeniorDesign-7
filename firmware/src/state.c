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

volatile enum STATES state = STATE_MENU_NAVIGATION;
volatile uint8_t render = 0;

char* deck_names[MAX_DECKS];
struct deck main_deck;
uint8_t num_decks = 0;

/* TODO: Needs to get `ls` from SD card and fill array */
void get_deck_names(char* deck_names[MAX_DECKS])
{
    num_decks = 8;
    deck_names[0] = "BIO 10100";
    deck_names[1] = "PHIL 32200";
    deck_names[2] = "ECE 20002";
    deck_names[3] = "ANTH 33700";
    deck_names[4] = "ECE 47700";
    deck_names[5] = "CS 15900";
    deck_names[6] = "ECE 43700";
    deck_names[7] = "ECE 36900";
}

/* TODO: we will only have one deck based on (deck number or name?) */
struct deck get_deck(uint8_t deck_number)
{
    struct deck d = {0};
    /* BIO 10100 */
    if (deck_number == 0) {
        strcpy(d.cards[0].front, "What is the mitochrondria?");
        strcpy(d.cards[0].back, "The powerhouse of the cell!");

        strcpy(d.cards[1].front, "FRONT Testing BIO 1");
        strcpy(d.cards[1].back, "BACK of testing 1");

        strcpy(d.cards[2].front, "FRONT Testing BIO 2");
        strcpy(d.cards[2].back, "BACK of testing 2");
    } else if (deck_number == 1) {
        strcpy(d.cards[0].front, "Who was Socrates?");
        strcpy(d.cards[0].back, "An awesome guy :)");

        strcpy(d.cards[1].front, "FRONT Testing PHIL 1");
        strcpy(d.cards[1].back, "BACK of testing PHIL 1");

        strcpy(d.cards[2].front, "FRONT Testing PHIL 2");
        strcpy(d.cards[2].back, "BACK of testing PHIL 2");
    }
    return d;
}

/* This will always be running when there is no interrupt happening */
void state_machine()
{
    uint8_t curr_page = 0;
    for (;;) {
        delay_ms(10);
        switch (state) {
        case STATE_DOWNLOAD:
            __disable_irq();
            /* Download */
            __enable_irq();

            break;

        case STATE_MENU_NAVIGATION:
            /* Get deck names */
            if (deck_names[0] == NULL) {
                get_deck_names(deck_names);
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
                main_deck = get_deck(curr_deck_selection);
                get_deck_from_sd = 0;
            }
            if (!render) break;

            eink_clear(0xFF);
            draw_header(deck_names[curr_deck_selection]);
            char buf[MAX_BACK_SIZE]; /* Back is larger than front */
            if (f_b == FRONT) {
                snprintf(buf, MAX_FRONT_SIZE, "%s", main_deck.cards[curr_card_selection].front);
            }
            else if (f_b == BACK) {
                snprintf(buf, MAX_BACK_SIZE, "%s", main_deck.cards[curr_card_selection].back);
            }
            draw_string(20, 20, buf, BLACK);
            eink_render_framebuffer();
            render = 0;

            break;
        }
        __WFI();
    }
}
