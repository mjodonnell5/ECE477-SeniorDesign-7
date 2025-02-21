#include <stdint.h>
#include <stm32l432xx.h>

#include "../include/clock.h"

void clock_init()
{
    /* MSI is already on */
    /* CANNOT change freq if not ready */
    while(!(RCC->CR & RCC_CR_MSIRDY));

    /* Set to 16 MHz */
    RCC->CR &= ~RCC_CR_MSIRANGE;
    RCC->CR |= RCC_CR_MSIRANGE_8;

    while(!(RCC->CR & RCC_CR_MSIRDY));

    SystemCoreClock = 16000000;

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

    TIM2->CR1 |= TIM_CR1_CEN;
}

void delay_ms(uint32_t ms)
{
    uint32_t count;
    while (ms--) {
        count = 1000;  // Approximate count for 1 ms at 4 MHz
        while (count--) {
            __asm__("nop");  // Prevent compiler optimization
        }
    }}
