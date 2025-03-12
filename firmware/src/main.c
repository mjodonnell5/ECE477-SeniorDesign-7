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

    // eink_clear(0xFF);
    // eink_render_framebuffer();

    render_pending = 1;
    state_machine();

    eink_sleep();

    for (;;);
    return 0;
}
