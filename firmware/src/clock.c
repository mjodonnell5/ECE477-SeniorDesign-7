#include <stdint.h>
#include <stm32l432xx.h>

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
}

void delay_init()
{
    /* Use TIM2 and just compare values when delaying instead of generating interrupts */

    /* Configure for 1kHz aka 1ms period */
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->PSC = 160 - 1;
    TIM2->ARR = 100 - 1;
    // TIM2->CR1 |= TIM_CR1_CEN;

}

/* NOTE: Probably don't use systick for this, run a timer and compare values when
* delaying, no point in generating a ton of interrupts, will interfere with wfi
* as well. */
void delay_ms(uint32_t ms)
{
    TIM2->CR1 |= TIM_CR1_CEN;
    uint32_t start = TIM2->CNT;
    uint32_t end = TIM2->CNT + ms;
    if (end > TIM2->ARR) {
        end -= TIM2->ARR;
    }
    while (start < end);
    TIM2->CR1 &= ~TIM_CR1_CEN;
    // uint32_t count;
    // while (ms--) {
    //     count = 1000;  // Approximate count for 1 ms at 4 MHz
    //     while (count--) {
    //         __asm__("nop");  // Prevent compiler optimization
    //     }
    // }
}
