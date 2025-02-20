#include <stm32l432xx.h>

#include "../include/ui.h" /* FIXME: REMOVE THIS AFTER MOVING FRONT AND BACK MACROS */

volatile uint8_t curr_deck_selection = 0;
volatile uint8_t curr_card_selection = 0;
volatile uint8_t get_deck_from_sd = 0;
volatile uint8_t f_b = FRONT;

void button_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    /* Input */
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5);

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD3_1 | GPIO_PUPDR_PUPD4_1 | GPIO_PUPDR_PUPD5_1);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PB;

    EXTI->RTSR1 |= EXTI_RTSR1_RT3
                | EXTI_RTSR1_RT4 | EXTI_RTSR1_RT5;

    /* FIXME: Cannot do long press until we do debouncing */
    /* Also set falling edge */
    // EXTI->FTSR1 |= EXTI_FTSR1_FT5;

    EXTI->IMR1 |= EXTI_IMR1_IM0 | EXTI_IMR1_IM5
               | EXTI_IMR1_IM3 | EXTI_IMR1_IM4;

    NVIC->ISER[0] |= (1 << EXTI3_IRQn) | (1 << EXTI4_IRQn) | (1 << EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF5) {
        EXTI->PR1 = EXTI_PR1_PIF5;
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
}

void EXTI4_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF4;
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

void EXTI3_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF3;
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

