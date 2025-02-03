#include <stm32l432xx.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"

int main(void)
{
    spi1_init();

    // for (;;) {
    //     eink_send_data(0xAA);
    // }

    eink_init();

    delay_ms(10);

    // eink_turn_on_display();

    eink_clear();

    // eink_draw_hline(50, 50, 100, BLACK_PIXEL);

    // eink_render_framebuffer();

    // eink_sleep();

    // for (;;);
    return 0;
}
