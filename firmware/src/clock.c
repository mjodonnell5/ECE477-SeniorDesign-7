#include <stdint.h>
#include <stm32l432xx.h>

#include "../include/clock.h"

void clock_init()
{
    /* Can ONLY change freq when MSI is READY if its on */
    while(!(RCC->CR & RCC_CR_MSIRDY));

    /* Allow CR to select range instead of CSR */
    RCC->CR |= RCC_CR_MSIRGSEL;

    /* Set to 16 MHz */
    RCC->CR &= ~RCC_CR_MSIRANGE;
    RCC->CR |= RCC_CR_MSIRANGE_8;

    SystemCoreClockUpdate();

    /* Setup delay_ms() */
    delay_init();
}

void delay_init()
{
    /* Use TIM2 and just compare values when delaying instead of generating interrupts
     * with SysTick */

    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    TIM2->CR1 &= ~TIM_CR1_CEN;

    /* 1 tick == 1ms */
    TIM2->PSC = 16000 - 1;
    TIM2->ARR = 0xFFFFFFFF - 1;

    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = TIM2->CNT;

    while ((uint32_t)(TIM2->CNT - start) < (ms));
}
