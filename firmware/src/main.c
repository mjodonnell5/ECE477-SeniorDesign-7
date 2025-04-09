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

/* Taken from ChatGPT */
uint16_t dec_to_bcd(uint16_t val) {
    return ((val / 10) << 4) | (val % 10);
}

/* Taken from ChatGPT */
uint16_t bcd_to_dec(uint16_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

struct dt {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint16_t year;
    uint8_t mon;
    uint8_t day;
};

void rtc_init()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_RTCAPBEN;

    /* Enable write access to RTC */
    PWR->CR1 |= PWR_CR1_DBP;

    /* Unlock write access to RTC registers */
    RTC->WPR = 0xCA;
    /* Do we need a delay? */
    delay_ms(5);
    RTC->WPR = 0x53;

    /* Calendar init */
    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF)) {}

    /* Program prescalars */
    RTC->PRER = 127 << RTC_PRER_PREDIV_A_Pos;
    RTC->PRER = 255 << RTC_PRER_PREDIV_S_Pos;

    /* Set the time */
    /* 24 hour time */
    RTC->TR &= ~RTC_TR_PM;
    /* Hour */
    uint16_t hour = dec_to_bcd(16);
    RTC->TR |= ((hour & 0xF0) << RTC_TR_HT_Pos) | ((hour & 0x0F) << RTC_TR_HU_Pos);
    /* Minute */
    uint16_t min = dec_to_bcd(30);
    RTC->TR |= ((min & 0xF0) << RTC_TR_MNT_Pos) | ((min & 0x0F) << RTC_TR_MNU_Pos);
    /* Second */
    uint16_t sec = dec_to_bcd(30);
    RTC->TR |= ((sec & 0xF0) << RTC_TR_ST_Pos) | ((sec & 0x0F) << RTC_TR_SU_Pos);

    /* Year */
    uint16_t year = dec_to_bcd(2025);
    RTC->TR |= ((year & 0xF0) << RTC_DR_YT_Pos) | ((year & 0x0F) << RTC_DR_YU_Pos);
    /* Month */
    uint16_t mon = dec_to_bcd(30);
    RTC->TR |= ((mon & 0xF0) << RTC_DR_MT_Pos) | ((mon & 0x0F) << RTC_DR_MU_Pos);
    /* Day */
    uint16_t day = dec_to_bcd(30);
    RTC->TR |= ((day & 0xF0) << RTC_DR_DT_Pos) | ((day & 0x0F) << RTC_DR_DU_Pos);

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;
}

void read_rtc(struct dt* dt)
{
    uint32_t tr = RTC->TR;
    uint32_t dr = RTC->DR;

    dt->hour = (((tr & RTC_TR_HT) >> RTC_TR_HT_Pos) * 10) + ((tr & RTC_TR_HU) >> RTC_TR_HU_Pos);
    dt->min = (((tr & RTC_TR_MNT) >> RTC_TR_MNT_Pos) * 10) + ((tr & RTC_TR_MNU) >> RTC_TR_MNU_Pos);
    dt->sec = (((tr & RTC_TR_ST) >> RTC_TR_ST_Pos) * 10) + ((tr & RTC_TR_SU) >> RTC_TR_SU_Pos);
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

    struct dt dt;
    rtc_init();
    char time[50];
    read_rtc(&dt);
    snprintf(time, 50, "%d:%d:%d", dt.hour, dt.day, dt.sec);
    draw_centered_string_wrapped(large_font, time, BLACK);
    eink_render_framebuffer();

    delay_ms(5000);
    read_rtc(&dt);
    snprintf(time, 50, "%d:%d:%d", dt.hour, dt.day, dt.sec);
    draw_centered_string_wrapped(large_font, time, BLACK);
    eink_render_framebuffer();

    for(;;);

    // render_pending = 1;
    // state_machine();

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
