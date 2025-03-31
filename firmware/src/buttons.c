#include <stm32l432xx.h>

#include "../include/ui.h" /* FIXME: REMOVE THIS AFTER MOVING FRONT AND BACK MACROS */
#include "../include/state.h"
#include "../include/buttons.h"

volatile uint8_t curr_deck_selection = 0;
volatile uint8_t curr_card_selection = 0;
volatile uint8_t get_deck_from_sd = 0;
volatile uint8_t f_b = FRONT;

extern uint8_t num_decks;
extern uint8_t curr_page;
extern struct deck main_deck;

void button_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* Input */
    GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE0);
    GPIOA->MODER &= ~(GPIO_MODER_MODE1);

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 |  GPIO_PUPDR_PUPD0);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD6_1 | GPIO_PUPDR_PUPD0_1);

    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD1);
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPD1_1);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PA;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PB;

    EXTI->RTSR1 |= EXTI_RTSR1_RT6
                | EXTI_RTSR1_RT1 | EXTI_RTSR1_RT0;

    /* FIXME: Cannot do long press until we do debouncing */
    /* Also set falling edge */
    /* FIXME: We only want long press on the back button */
    // EXTI->FTSR1 |= EXTI_FTSR1_FT6
    //             | EXTI_FTSR1_FT2 | EXTI_FTSR1_FT0;
    EXTI->FTSR1 |= EXTI_FTSR1_FT0 | EXTI_FTSR1_FT1;

    EXTI->IMR1 |= EXTI_IMR1_IM0
               | EXTI_IMR1_IM6 | EXTI_IMR1_IM1;

    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn) | (1 << EXTI0_IRQn) | (1 << EXTI1_IRQn);
}

volatile uint32_t start_press_time = 0;

volatile uint8_t press = NO_PRESS;
volatile uint8_t btn = SELECT;

/* SELECT/FLIP */
void EXTI0_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF0;

    if (render_pending) {
        return;
    }

    btn = SELECT;
    if (GPIOB->IDR & GPIO_IDR_ID0) {
        /* Rising edge */
        start_press_time = TIM2->CNT;
        return;
    } else {
        /* Falling edge */
        if ((uint32_t)(TIM2->CNT - start_press_time) < LONG_PRESS_TIME_MS) {
            /* Short press */
            press = SHORT_PRESS;
        } else {
            /* Long press */
            press = LONG_PRESS;
        }
    }

    render_pending = 1;
}

/* DOWN/BACKWARDS */
void EXTI1_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF1;
    if (render_pending) {
        return;
    }
    btn = BACKWARD;
    if (GPIOA->IDR & GPIO_IDR_ID1) {
        /* Rising edge */
        start_press_time = TIM2->CNT;
        return;
    } else {
        /* Falling edge */
        if ((uint32_t)(TIM2->CNT - start_press_time) < LONG_PRESS_TIME_MS) {
            /* Short press */
            press = SHORT_PRESS;
        } else {
            /* Long press */
            press = LONG_PRESS;
        }
    }
    render_pending = 1;
}

/* UP/FORWARDS */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF6) {
        EXTI->PR1 = EXTI_PR1_PIF6;
        if (render_pending) {
            return;
        }
        btn = FORWARD;
        press = SHORT_PRESS;

        render_pending = 1;
    }
}
