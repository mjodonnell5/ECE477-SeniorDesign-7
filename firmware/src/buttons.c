#include <stm32l496xx.h>

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

// void disable_buttons()
// {
//     EXTI->IMR1 &= ~(EXTI_IMR1_IM0
//                | EXTI_IMR1_IM6 | EXTI_IMR1_IM1);
// }
//
// void enable_buttons()
// {
//     EXTI->IMR1 |= EXTI_IMR1_IM0
//                | EXTI_IMR1_IM6 | EXTI_IMR1_IM1;
// }

void button_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;

    /* Input: B1 - PE5 ; B2 - PE3; B3 - PE4 */
    GPIOE->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4);

    // GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD3 |  GPIO_PUPDR_PUPD4);
    // GPIOE->PUPDR |= (GPIO_PUPDR_PUPD5_1 | GPIO_PUPDR_PUPD3_1 | GPIO_PUPDR_PUPD4_1);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PE;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PE;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PE;

    EXTI->RTSR1 |= EXTI_RTSR1_RT5 | EXTI_RTSR1_RT3 | EXTI_RTSR1_RT4;

    EXTI->FTSR1 |= EXTI_FTSR1_FT5 | EXTI_FTSR1_FT3;

    EXTI->IMR1 |= EXTI_IMR1_IM5 | EXTI_IMR1_IM3 | EXTI_IMR1_IM4;

    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn) | (1 << EXTI3_IRQn) | (1 << EXTI4_IRQn);
    // NVIC_EnableIRQ(EXTI3_IRQn);
    // NVIC_EnableIRQ(EXTI4_IRQn);
}

volatile uint32_t start_press_time = 0;

volatile uint8_t press = NO_PRESS;
volatile uint8_t btn = SELECT;

/* SELECT/FLIP */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF5) {
        EXTI->PR1 = EXTI_PR1_PIF5;
        if (render_pending) {
            return;
        }

        btn = SELECT;
        if (GPIOE->IDR & GPIO_IDR_ID5) {
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
}


/* DOWN/BACKWARDS */
void EXTI3_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF3;
    if (render_pending) {
        return;
    }

    btn = BACKWARD;
    if (GPIOE->IDR & GPIO_IDR_ID3) {
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
void EXTI4_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF4;

    if (render_pending) {
        return;
    }
    btn = FORWARD;
    press = SHORT_PRESS;

    render_pending = 1;
}
