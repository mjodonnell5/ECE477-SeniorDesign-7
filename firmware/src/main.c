#include <stm32l432xx.h>
#include <string.h>
#include <stdio.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"
#include "../include/ui.h"

enum STATES {
    STATE_DOWNLOAD = 0, /* DISABLE INTERRUPTS */
    STATE_MENU_NAVIGATION,
    STATE_FLASHCARD_NAVIGATION,
    STATE_DECK_SELECTION
};

void draw_test()
{
    draw_rect(0, 0, 265, 80, BLACK);

    for (uint8_t i = 0; i < 32; ++i) {
        draw_char(5 + (i * 8), 10, '!' + i, BLACK);
    }

    for (uint8_t i = 0; i < 26; ++i) {
        draw_char(5 + (i * 8), 20, 'A' + i, BLACK);
    }

    for (uint8_t i = 0; i < 26; ++i) {
        draw_char(5 + (i * 8), 30, 'a' + i, BLACK);
    }

    draw_string(5, 50, "IT WORKS, LETS GO!!! :)", BLACK);
    draw_string(20, 65, "-Cole", BLACK);
}


volatile enum STATES state = STATE_MENU_NAVIGATION;

/* NOTE: This will be set to 1 in the GPIO button interrupts. This will make sure that
 * we do not queue a bunch of renders back to back. If buttons are spammed this will just
 * continue being set, therefore queueing only a single render. This will ONLY be cleared
 * once the current render is completed */
volatile uint8_t render = 0;


char* deck_names[6];
struct deck main_deck;

/* TODO: Needs to get `ls` from SD card and fill array */
void get_deck_names(char* deck_names[6])
{
    deck_names[0] = "BIO 10100";
    deck_names[1] = "PHIL 32200";
    deck_names[2] = "ECE 20002";
    deck_names[3] = "ANTH 33700";
    deck_names[4] = "ECE 47700";
    deck_names[5] = "CS 15900";
}

/* TODO: we will only have one deck based on (deck number or name?) */
struct deck get_deck(uint8_t deck_number)
{
    struct deck d = {0};
    /* BIO 10100 */
    if (deck_number == 0) {
        strcpy(d.cards[0].front, "What is the mitochrondria?");
        strcpy(d.cards[0].back, "The powerhouse of the cell!");

        strcpy(d.cards[1].front, "Testing BIO 1");
        strcpy(d.cards[1].back, "BACK of testing 1");

        strcpy(d.cards[2].front, "Testing BIO 2");
        strcpy(d.cards[2].back, "BACK of testing 2");
    } else if (deck_number == 1) {
        strcpy(d.cards[0].front, "Who was Socrates?");
        strcpy(d.cards[0].back, "An awesome guy :)");

        strcpy(d.cards[1].front, "Testing PHIL 1");
        strcpy(d.cards[1].back, "BACK of testing PHIL 1");

        strcpy(d.cards[2].front, "Testing PHIL 2");
        strcpy(d.cards[2].back, "BACK of testing PHIL 2");
    }
    return d;
}


/* This will always be running when there is no interrupt happening */
void state_machine()
{
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

            eink_clear(0xFF);
            draw_main_menu(curr_deck_selection, deck_names, 6);
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
    }
}

int main(void)
{
    spi1_init();

    eink_init();

    button_init();

    // #define WRAP_TEST
    #ifdef WRAP_TEST
    eink_clear(0xFF);
    char str[150] = "Hello, this is a test of the wrapping on the display :) lkasdflaskldfj kasdkjl asdflk jj mm fjfjjf fjjfjfjf jjfjf";
    // char str[100] = "Hello :)";
    draw_string_wrapped(0, 0, str, BLACK);
    draw_string(0, 35, str, BLACK);
    eink_render_framebuffer();
    #endif /* WRAP_TEST */

    // #define FLASHCARD_FRONT
    #ifdef FLASHCARD_FRONT
    eink_clear(0xFF);
    struct flashcard fc;
    char front[250] = "MITOCHONDRIA";
    // char back[300] = "It is the powerhouse of the cell. This is learned in Biology class.";
    char back[300] = "Biology class. asdflk jlkasjdf alksdjf kljasdg f";
    strcpy(fc.front, front);
    strcpy(fc.back, back);
    draw_flashcard(fc, 1, BLACK);
    eink_render_framebuffer();
    #endif /* FLASHCARD_FRONT */

    // #define CLEAR
    #ifdef CLEAR
    eink_clear(0xFF);
    eink_render_framebuffer();
    #endif /* CLEAR */

    #define STATE_MACHINE
    #ifdef STATE_MACHINE
    render = 1;
    state_machine();
    #endif /* STATE_MACHINE */

    // #define SHIFT
    #ifdef SHIFT
    for (uint8_t i = 0; i < 6; ++i) {
        eink_clear(0xFF);

        draw_main_menu(i);

        eink_render_framebuffer();
    }
    #endif /* STATE */

    eink_sleep();

    for (;;);
    return 0;
}
