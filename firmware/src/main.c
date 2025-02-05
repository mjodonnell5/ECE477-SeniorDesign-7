#include <stm32l432xx.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"

int main(void)
{
    spi1_init();

    eink_init();

    /* Set framebuffer to all white */
    eink_clear();

    eink_draw_rect(0, 0, 265, 80, BLACK);

    for (uint8_t i = 0; i < 32; ++i) {
        eink_draw_char(5 + (i * 8), 10, '!' + i, BLACK);
    }

    for (uint8_t i = 0; i < 26; ++i) {
        eink_draw_char(5 + (i * 8), 20, 'A' + i, BLACK);
    }

    for (uint8_t i = 0; i < 26; ++i) {
        eink_draw_char(5 + (i * 8), 30, 'a' + i, BLACK);
    }

    eink_draw_string(5, 50, "IT WORKS, LETS GO!!! :)", BLACK);
    eink_draw_string(20, 65, "-Cole", BLACK);

    eink_render_framebuffer();

    eink_sleep();

    for (;;);
    return 0;
}
