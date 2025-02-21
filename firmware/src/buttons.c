#include <stm32l432xx.h>

#include "../include/ui.h" /* FIXME: REMOVE THIS AFTER MOVING FRONT AND BACK MACROS */
#include "../include/state.h"

volatile uint8_t curr_deck_selection = 0;
volatile uint8_t curr_card_selection = 0;
volatile uint8_t get_deck_from_sd = 0;
volatile uint8_t f_b = FRONT;

extern uint8_t num_decks;
extern uint8_t curr_page;

void button_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* Input */
    GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE0);
    GPIOA->MODER &= ~(GPIO_MODER_MODE2);

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 |  GPIO_PUPDR_PUPD0);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD6_1 | GPIO_PUPDR_PUPD0_1);

    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2);
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPD2_1);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PA;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PB;

    EXTI->RTSR1 |= EXTI_RTSR1_RT6
                | EXTI_RTSR1_RT2 | EXTI_RTSR1_RT0;

    /* FIXME: Cannot do long press until we do debouncing */
    /* Also set falling edge */
    // EXTI->FTSR1 |= EXTI_FTSR1_FT5;

    EXTI->IMR1 |= EXTI_IMR1_IM0
               | EXTI_IMR1_IM6 | EXTI_IMR1_IM2;

    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn) | (1 << EXTI0_IRQn) | (1 << EXTI2_IRQn);
}

void EXTI0_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF0;
    if (render) {
        return;
    }
    if (state == STATE_FLASHCARD_NAVIGATION) {
        /* Flip */
        f_b = !f_b;
    } else {
        state = STATE_FLASHCARD_NAVIGATION;
        get_deck_from_sd = 1;
    }
    render = 1;
}

void EXTI2_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF2;
    if (render) {
        return;
    }
    if (state == STATE_MENU_NAVIGATION) {
        curr_deck_selection = (curr_deck_selection - 1);  
    } else if (state == STATE_FLASHCARD_NAVIGATION) {
        curr_card_selection = (curr_card_selection - 1);  
        if (f_b == BACK) f_b = FRONT;
    }
    render = 1;
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF6) {
        EXTI->PR1 = EXTI_PR1_PIF6;
        if (render) {
            return;
        }
        if (state == STATE_MENU_NAVIGATION) {
            curr_deck_selection = (curr_deck_selection + 1);  
        } else if (state == STATE_FLASHCARD_NAVIGATION) {
            curr_card_selection = (curr_card_selection + 1);  
            if (f_b == BACK) f_b = FRONT;
        }
        render = 1;
    }
}

