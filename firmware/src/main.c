#include <stm32l432xx.h>
#include <string.h>
#include <stdio.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/state.h"

#include "../include/commands.h"

int main(void)
{
    clock_init();

    spi1_init();

    eink_init();

    button_init();

    mount();

    // struct deck d = {0};

    // static char decks[MAX_DECKS][MAX_NAME_SIZE] = {0};

    // parseJSON_file("test.txt", &d);
    // get_decks(decks);

    // eink_clear(0xFF);
    // log_to_sd("FIRST DECK");
    // log_to_sd(decks[1]);
    // draw_string(0, 0, decks[2], BLACK);
    // draw_string(0, 10, decks[3], BLACK);
    // draw_string(0, 20, decks[4], BLACK);
    // draw_string(0, 30, decks[5], BLACK);
    // draw_string(0, 0, d.cards[0].front, BLACK);
    // draw_string_wrapped(15, 15, d.cards[0].back, BLACK);
    // eink_render_framebuffer();

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
    // draw_centered_string_wrapped("Condensation", BLACK);
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
