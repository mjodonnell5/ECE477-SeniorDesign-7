#include <stm32l432xx.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"

int main(void)
{
    spi1_init();

    // for (;;) {
    //     eink_send_data(0x55);
    //     // GPIOA->BSRR = GPIO_BSRR_BR4;
    //     //
    //     // delay_ms(1);
    //     //
    //     // spi_tx(SPI1, 0xAA);
    //     //
    //     // delay_ms(1);
    //     //
    //     // GPIOA->BSRR = GPIO_BSRR_BS4;
    //     //
    //     // delay_ms(1);
    // }

    // eink_init();

    // eink_draw_hline(50, 50, 100, WHITE_PIXEL);
    //
    // eink_sleep();

    for (;;);
    return 0;
}
