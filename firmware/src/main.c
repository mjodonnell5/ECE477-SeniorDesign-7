#include <stdio.h>
#include <string.h>
#include <stm32l432xx.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/state.h"
#include "../include/uart.h"
#include "../include/commands.h"

int main(void)
{
    clock_init();

    spi1_init();

    eink_init();

    button_init();

    uart_init();

    mount();

    // #define WRAP_TEST
    #ifdef WRAP_TEST
    eink_clear(0xFF);
    char str[150] = "Hello, this is a test of the wrapping on the display :) lkasdflaskldfj kasdkjl asdflk jj mm fjfjjf fjjfjfjf jjfjf";
    // char str[100] = "Hello :)";
    draw_string_wrapped(0, 0, str, BLACK);
    draw_string(0, 35, str, BLACK);
    eink_render_framebuffer();
    #endif /* WRAP_TEST */

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
