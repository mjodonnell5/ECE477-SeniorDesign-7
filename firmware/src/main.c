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

    eink_draw_hline(50, 50, 100, BLACK);

    eink_render_framebuffer();

    eink_sleep();

    for (;;);
    return 0;
}
