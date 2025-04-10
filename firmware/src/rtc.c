#include <stdint.h>
#include <stm32l496xx.h>

#include "../include/clock.h"
#include "../include/rtc.h"

/* Taken from ChatGPT */
uint16_t dec_to_bcd(uint16_t val) {
    return ((val / 10) << 4) | (val % 10);
}

/* Taken from ChatGPT */
uint16_t bcd_to_dec(uint16_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

void rtc_init()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_RTCAPBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;

    RCC->CSR |= RCC_CSR_LSION;
    while (!(RCC->CSR & RCC_CSR_LSIRDY));

    /* Enable write access to RTC */
    PWR->CR1 |= PWR_CR1_DBP;

    /* Reset backup domain */
    RCC->BDCR |= RCC_BDCR_BDRST;
    RCC->BDCR &= ~RCC_BDCR_BDRST;

    /* Set RTC clock to LSI */
    RCC->BDCR &= ~RCC_BDCR_RTCSEL;
    RCC->BDCR |= RCC_BDCR_RTCSEL_1;

    RCC->BDCR |= RCC_BDCR_RTCEN;

    /* Unlock write access to RTC registers */
    RTC->WPR = 0xCA;
    /* Do we need a delay? */
    delay_ms(5);
    RTC->WPR = 0x53;

    /* Calendar init */
    RTC->ISR |= RTC_ISR_INIT;
    while (!(RTC->ISR & RTC_ISR_INITF));

    /* Program prescalars (for 32khz [LSI]) */
    RTC->PRER = (127 << RTC_PRER_PREDIV_A_Pos) | (255 << RTC_PRER_PREDIV_S_Pos);

    /* 24 hour time */
    RTC->TR &= ~RTC_TR_PM;

    /* Set the time and date */
    uint16_t hour = dec_to_bcd(15);
    uint16_t min = dec_to_bcd(20);
    uint16_t sec = dec_to_bcd(35);
    RTC->TR = (((hour & 0xF0) >> 4) << RTC_TR_HT_Pos) | ((hour & 0x0F) << RTC_TR_HU_Pos) |
              (((min & 0xF0) >> 4) << RTC_TR_MNT_Pos) | ((min & 0x0F) << RTC_TR_MNU_Pos) |
              (((sec & 0xF0) >> 4) << RTC_TR_ST_Pos) | ((sec & 0x0F) << RTC_TR_SU_Pos);

    uint16_t year = dec_to_bcd(25);
    uint16_t mon = dec_to_bcd(4);
    uint16_t day = dec_to_bcd(10);
    RTC->DR = (((year & 0xF0) >> 4) << RTC_DR_YT_Pos) | ((year & 0x0F) << RTC_DR_YU_Pos) |
              (((mon & 0xF0) >> 4) << RTC_DR_MT_Pos) | ((mon & 0x0F) << RTC_DR_MU_Pos) |
              (((day & 0xF0) >> 4) << RTC_DR_DT_Pos) | ((day & 0x0F) << RTC_DR_DU_Pos);

    RTC->ISR &= ~RTC_ISR_INIT;
    RTC->WPR = 0xFF;

    /* These don't seem to be working */
    while (!(RTC->ISR & RTC_ISR_INITS));
    while (!(RTC->ISR & RTC_ISR_RSF));
}

void read_rtc(struct dt* dt)
{
    uint32_t tr = RTC->TR;
    uint32_t dr = RTC->DR;

    dt->hour = (((tr & RTC_TR_HT) >> RTC_TR_HT_Pos) * 10) + (((tr & RTC_TR_HU) >> RTC_TR_HU_Pos));
    dt->min = (((tr & RTC_TR_MNT) >> RTC_TR_MNT_Pos) * 10) + (((tr & RTC_TR_MNU) >> RTC_TR_MNU_Pos));
    dt->sec = (((tr & RTC_TR_ST) >> RTC_TR_ST_Pos) * 10) + (((tr & RTC_TR_SU) >> RTC_TR_SU_Pos));

    dt->year = (((dr & RTC_DR_YT) >> RTC_DR_YT_Pos) * 10) + (((dr & RTC_DR_YU) >> RTC_DR_YU_Pos));
    dt->mon = (((dr & RTC_DR_MT) >> RTC_DR_MT_Pos) * 10) + (((dr & RTC_DR_MU) >> RTC_DR_MU_Pos));
    dt->day = (((dr & RTC_DR_DT) >> RTC_DR_DT_Pos) * 10) + (((dr & RTC_DR_DU) >> RTC_DR_DU_Pos));
}
