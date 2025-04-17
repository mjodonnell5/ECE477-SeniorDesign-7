#include <stdio.h>
#include <string.h>
#include <stm32l496xx.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"
#include "../include/ui.h"
#include "../include/buttons.h"
#include "../include/state.h"
#include "../include/uart.h"
#include "../include/commands.h"
#include "../include/battery.h"
#include "../include/rtc.h"

int main(void)
{
    clock_init();

    spi1_init();

    eink_init();

    button_init();

    uart_init();

    charge_init();

    rtc_init();

    /* Allow us to use GPIOG (needed for SD card) */
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    PWR->CR2 |= PWR_CR2_IOSV;

    mount();


    // eink_clear(0xFF);
    // draw_sleep_image(35, 35);
    // eink_render_framebuffer();
    // draw_centered_string_wrapped(large_font, "HI :))))", BLACK);
    // eink_render_framebuffer();
    // for(;;);
    //
    // eink_clear(0xFF);
    // eink_render_framebuffer();

    // char time[50];
    // read_rtc(&dt);
    // snprintf(time, 50, "20%02d-%02d-%02d | %02d:%02d:%02d", dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);
    // eink_clear(0xFF);
    // draw_centered_string_wrapped(large_font, time, BLACK);
    // eink_render_framebuffer();
    //
    // delay_ms(3000);
    // read_rtc(&dt);
    // snprintf(time, 50, "20%02d-%02d-%02d | %02d:%02d:%02d", dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);
    // eink_clear(0xFF);
    // draw_centered_string_wrapped(large_font, time, BLACK);
    // eink_render_framebuffer();
    //
    // for(;;);


    /* FIXME: Set initial font, this is a bad place to do it but the code is messy
     * and I don't know where to do it lol */
    curr_font = xlarge_font;
    render_pending = 1;
    state_machine();

    // eink_sleep();

    GPIO_Init();
    I2C1_Init();


    uint16_t voltage = BQ27441_ReadWord(BQ27441_COMMAND_VOLTAGE);
    uint16_t soc = BQ27441_ReadWord(BQ27441_COMMAND_SOC);
    uint16_t capacity = BQ27441_ReadWord(BQ27441_COMMAND_REM_CAPACITY);
    uint16_t fullavail = BQ27441_ReadWord(0x0A);
    uint16_t remcap = BQ27441_ReadWord(0x0C);
    uint16_t fullchargecap = BQ27441_ReadWord(0x0E);

    char log_buffer[50];

    sprintf(log_buffer, "Voltage: %.3f V\n", voltage / 1000.0);
    log_to_sd(log_buffer);

    sprintf(log_buffer, "State of Charge: %d%%\n", soc);
    log_to_sd(log_buffer);

    sprintf(log_buffer, "Remaining Capacity: %d mAh\n", capacity);
    log_to_sd(log_buffer);

    sprintf(log_buffer, "Full Avail Cap: %d MaH\n", fullavail);
    log_to_sd(log_buffer);

    sprintf(log_buffer, "Rem Cap: %d MaH\n", remcap);
    log_to_sd(log_buffer);
    sprintf(log_buffer, "Full Charge Cap: %d MaH\n", fullchargecap);
    log_to_sd(log_buffer);

    for (;;);
    return 0;
}
