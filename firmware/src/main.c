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

    delay_ms(20);

    // eink_turn_on_display();

    eink_clear();

    // delay_ms(5000);

    // eink_draw_hline(50, 50, 100, BLACK_PIXEL);

    // delay_ms(100);

    // eink_render_framebuffer();

    eink_sleep();

    for (;;);
    return 0;
}
