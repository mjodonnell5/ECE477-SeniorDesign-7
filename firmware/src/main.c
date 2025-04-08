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

/* Any EXTI line setup for interrupts will wakeup from stop 2 mode */
void enter_stop_2()
{
    PWR->CR1 &= ~PWR_CR1_LPMS;
    PWR->CR1 |= PWR_CR1_LPMS_STOP2;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __WFI();
}

/* This will be falling edge of power switch interrupt */
void power_off()
{
    /* Disable button interrupts; we ONLY want the power switch to be able to wakeup */
    disable_buttons();

    eink_sleep();

    /* Buttons */
    /* Set to analog */
    /* Disable pull up/down */
    RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOAEN;

    /* E-ink */
    RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;

    /* SD card */
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_SPI3EN;

    /* Battery monitor */

    enter_stop_2();
}

/* This will be rising edge of power switch interrupt */
void wake_up()
{
    /* Upon wake up we just want to reset so that we can run the main function 
     * again, we could just call all the functions again but this is simpler I think*/
    NVIC_SystemReset();
}

int main(void)
{
    clock_init();

    spi1_init();

    eink_init();

    button_init();

    uart_init();

    // mount();
    // log_to_sd("mounted");


    // eink_clear(0xFF);
    // draw_sleep_image(35, 35);
    // eink_render_framebuffer();
    // draw_centered_string_wrapped(large_font, "HI :))))", BLACK);
    // eink_render_framebuffer();

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
