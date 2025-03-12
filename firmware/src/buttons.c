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
    /* FIXME: We only want long press on the back button */
    // EXTI->FTSR1 |= EXTI_FTSR1_FT6
    //             | EXTI_FTSR1_FT2 | EXTI_FTSR1_FT0;

    EXTI->IMR1 |= EXTI_IMR1_IM0
               | EXTI_IMR1_IM6 | EXTI_IMR1_IM2;

    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn) | (1 << EXTI0_IRQn) | (1 << EXTI2_IRQn);
}

volatile uint32_t start_press_time = 0;
void EXTI0_IRQHandler(void)
{
    if (GPIOB->IDR & GPIO_IDR_ID0) {
        /* Rising edge */
        /* Start long press count */
        start_press_time = TIM2->CNT;
    } else {
        /* Falling edge */
        if ((uint32_t)(TIM2->CNT - start_press_time) < LONG_PRESS_TIME_MS) {
            /* Short press */
        } else {
            /* Long press */
        }

    }

    /* FIXME: Maybe name this mutex something better? */
    if (render_pending) {
        return;
    }
    if (state == STATE_FLASHCARD_NAVIGATION) {
        /* Flip */
        f_b = !f_b;
    } else {
        state = STATE_FLASHCARD_NAVIGATION;
        get_deck_from_sd = 1;
    }
    render_pending = 1;
    EXTI->PR1 = EXTI_PR1_PIF0;
}

void EXTI2_IRQHandler(void)
{
    if (GPIOA->IDR & GPIO_IDR_ID2) {
        /* Rising edge */

    } else {
        /* Falling edge */

    }
    if (render_pending) {
        return;
    }
    if (state == STATE_MENU_NAVIGATION) {
        if (curr_deck_selection > 0) {
            curr_deck_selection--;
        }
    } else if (state == STATE_FLASHCARD_NAVIGATION) {
        if (curr_card_selection > 0) {
            curr_card_selection--;
        }
        if (f_b == BACK) f_b = FRONT;
    }
    render_pending = 1;
    EXTI->PR1 = EXTI_PR1_PIF2;
}

void EXTI9_5_IRQHandler(void)
{
    if (GPIOB->IDR & GPIO_IDR_ID6) {
        /* Rising edge */

    } else {
        /* Falling edge */

    }
    if (EXTI->PR1 & EXTI_PR1_PIF6) {
        if (render_pending) {
            return;
        }
        if (state == STATE_MENU_NAVIGATION) {
            if (curr_deck_selection + 1 < num_decks) {
                curr_deck_selection++;
            }
        } else if (state == STATE_FLASHCARD_NAVIGATION) {
            if (curr_card_selection + 1 < main_deck.num_cards) {
                curr_card_selection++;
            }
            if (f_b == BACK) f_b = FRONT;
        }
        render_pending = 1;
        EXTI->PR1 = EXTI_PR1_PIF6;
    }
}

