#include <stm32l496xx.h>

#include "../include/ui.h" /* FIXME: REMOVE THIS AFTER MOVING FRONT AND BACK MACROS */
#include "../include/state.h"
#include "../include/buttons.h"
#include "../include/eink.h"
#include "../include/clock.h"

volatile uint8_t curr_deck_selection = 0;
volatile uint8_t curr_card_selection = 0;
volatile uint8_t get_deck_from_sd = 0;
volatile uint8_t f_b = FRONT;

extern uint8_t num_decks;
extern uint8_t curr_page;
extern struct deck main_deck;

void disable_buttons()
{
    EXTI->IMR1 &= ~(EXTI_IMR1_IM3
               | EXTI_IMR1_IM4 | EXTI_IMR1_IM5);
}

void enable_buttons()
{
    EXTI->IMR1 |= EXTI_IMR1_IM3
               | EXTI_IMR1_IM4 | EXTI_IMR1_IM5;
}

void power_button_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
    GPIOE->MODER &= ~GPIO_MODER_MODE2;
    GPIOE->PUPDR &= ~(GPIO_PUPDR_PUPD2);
    GPIOE->PUPDR |= (GPIO_PUPDR_PUPD2_1);
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PE;

    EXTI->RTSR1 |= EXTI_RTSR1_RT2;

    EXTI->FTSR1 |= EXTI_FTSR1_FT2;

    EXTI->IMR1 |= EXTI_IMR1_IM2;

    NVIC->ISER[0] |= 1 << EXTI2_IRQn;
}

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
    power_button_init();
    // NVIC_EnableIRQ(EXTI3_IRQn);
    // NVIC_EnableIRQ(EXTI4_IRQn);
}

volatile uint32_t start_press_time = 0;

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
    // disable_buttons();

    eink_clear(0xFF);
    draw_header("SLEEPING");
    draw_string(large_font, 100, 50, "START STUDYING AGAIN SOON!", BLACK);
    draw_sleep_image(100, 75);
    eink_render_framebuffer();

    eink_sleep();

    /* Buttons */
    /* Set to analog */
    /* Disable pull up/down */
    // RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOBEN;
    // RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOAEN;

    /* E-ink */
    // RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;

    /* SD card */
    // RCC->APB1ENR1 &= ~RCC_APB1ENR1_SPI3EN;

    /* Battery monitor */

    // enter_stop_2();
}

/* This will be rising edge of power switch interrupt */
void wake_up()
{
    /* Upon wake up we just want to reset so that we can run the main function 
     * again, we could just call all the functions again but this is simpler I think*/
    enable_buttons();

    // eink_clear(0xFF);
    // draw_centered_string_wrapped(xlarge_font, "E-ink Flashcards (logo somewhere here?)", BLACK);
    // eink_render_framebuffer();
    //
    // delay_ms(1500);

    NVIC_SystemReset();
}

volatile uint8_t interrupts = 0;

void EXTI2_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF2;

    if (GPIOE->IDR & GPIO_IDR_ID2) {
        /* Rising edge */
        // power_off();
        state = STATE_SLEEPING;
        interrupts++;
        render_pending = 1;
    } else {
        /* Falling edge */
        wake_up();
    }
}


/* SELECT/FLIP */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF5) {
        EXTI->PR1 = EXTI_PR1_PIF5;
        // if (render_pending) {
        //     return;
        // }

        if (GPIOE->IDR & GPIO_IDR_ID5) {
            /* Rising edge */
            start_press_time = TIM2->CNT;
            return;
        } else {
            /* Falling edge */
            if ((uint32_t)(TIM2->CNT - start_press_time) < LONG_PRESS_TIME_MS) {
                /* Short press */
                evq_push(EVENT_BUTTON_SEL_SHORT);
            } else {
                /* Long press */
                evq_push(EVENT_BUTTON_SEL_LONG);
            }
        }

        render_pending = 1;
        interrupts++;

        /* BUSY LOW */
    } else if (EXTI->PR1 & EXTI_PR1_PIF6) {
        EXTI->PR1 = EXTI_PR1_PIF6;

        eink_busy = 0;
    }
}

/* DOWN/BACKWARDS */
void EXTI3_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF3;

    if (GPIOE->IDR & GPIO_IDR_ID3) {
        /* Rising edge */
        start_press_time = TIM2->CNT;
        return;
    } else {
        /* Falling edge */
        if ((uint32_t)(TIM2->CNT - start_press_time) < LONG_PRESS_TIME_MS) {
            /* Short press */
            evq_push(EVENT_BUTTON_DOWN_SHORT);
        } else {
            /* Long press */
            evq_push(EVENT_BUTTON_DOWN_LONG);
        }
    }
    render_pending = 1;
    interrupts++;
}

/* UP/FORWARDS */
void EXTI4_IRQHandler(void)
{
    EXTI->PR1 = EXTI_PR1_PIF4;

    // if (render_pending) {
    //     return;
    // }

    evq_push(EVENT_BUTTON_UP_SHORT);

    render_pending = 1;
    interrupts++;
}
