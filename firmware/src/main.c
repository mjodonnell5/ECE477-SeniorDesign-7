#include <stm32l432xx.h>
#include <string.h>
#include <stdio.h>

#include "../include/eink.h"
#include "../include/spi.h"
#include "../include/clock.h"
#include "../include/ui.h"

enum STATES {
    STATE_IDLE = 0,
    STATE_DOWNLOAD, /* DISABLE INTERRUPTS */
    STATE_MENU_NAVIGATION,
    STATE_FLASHCARD_NAVIGATION,
    STATE_DECK_SELECTION
};

void draw_test()
{
    draw_rect(0, 0, 265, 80, BLACK);

    for (uint8_t i = 0; i < 32; ++i) {
        draw_char(5 + (i * 8), 10, '!' + i, BLACK);
    }

    for (uint8_t i = 0; i < 26; ++i) {
        draw_char(5 + (i * 8), 20, 'A' + i, BLACK);
    }

    for (uint8_t i = 0; i < 26; ++i) {
        draw_char(5 + (i * 8), 30, 'a' + i, BLACK);
    }

    draw_string(5, 50, "IT WORKS, LETS GO!!! :)", BLACK);
    draw_string(20, 65, "-Cole", BLACK);
}


volatile enum STATES state = STATE_MENU_NAVIGATION;

/* NOTE: This will be set to 1 in the GPIO button interrupts. This will make sure that
 * we do not queue a bunch of renders back to back. If buttons are spammed this will just
 * continue being set, therefore queueing only a single render. This will ONLY be cleared
 * once the current render is completed */
volatile uint8_t render = 0;

volatile uint8_t button_presses = 0;
volatile uint8_t curr_deck_selection = 0;
volatile uint8_t curr_card_selection = 0;
volatile uint8_t f_b = FRONT;

char* deck_names[6] = {"BIO 10100", "PHIL 32200", "ECE 20002", "ANTH 33700", "ECE 47700", "CS 15900"};
struct flashcard decks[1][3];
void setup_deck()
{
    /* BIO 10100 */
    strcpy(decks[0][0].front, "MITOCHONDRIA");
    strcpy(decks[0][0].back, "The powerhouse of the cell");

    strcpy(decks[0][1].front, "Testing 1");
    strcpy(decks[0][1].back, "back of testing 1");

    strcpy(decks[0][2].front, "Testing 2");
    strcpy(decks[0][2].back, "back of testing 2");
}

/* This will always be running when there is no interrupt happening */
void state_machine()
{
    for (;;) {
        delay_ms(10);
        switch (state) {
        case STATE_IDLE:
            break;
        case STATE_DOWNLOAD:
            break;
        case STATE_MENU_NAVIGATION:
            /* FIXME: We need to draw everything to the framebuffer BEFORE
             * we render. */
            if (render) {
                eink_clear(0xFF);
                draw_main_menu(curr_deck_selection, deck_names, 6);
                eink_render_framebuffer();
                render = 0;
            }
            break;
        case STATE_DECK_SELECTION:
        case STATE_FLASHCARD_NAVIGATION:
            if (render) {
                eink_clear(0xFF);
                draw_header(deck_names[curr_deck_selection]);
                char buf[512];
                if (f_b == FRONT)
                    sprintf(buf, "%s", decks[curr_deck_selection][curr_card_selection].front);
                else if (f_b == BACK)
                    sprintf(buf, "%s", decks[curr_deck_selection][curr_card_selection].back);
                draw_string(20, 20, buf, BLACK);
                eink_render_framebuffer();
                render = 0;
            }
            break;
        }
    }
}

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

    EXTI->IMR1 |= EXTI_IMR1_IM0 | EXTI_IMR1_IM5
               | EXTI_IMR1_IM3 | EXTI_IMR1_IM4;

    NVIC->ISER[0] |= (1 << EXTI3_IRQn) | (1 << EXTI4_IRQn) | (1 << EXTI9_5_IRQn);
}

/* FIXME: I can have a "display variable" that stores the current item selected on the
 * display. If this differs from the internal state I can queue a render. */

void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR1 & EXTI_PR1_PIF5) {
        EXTI->PR1 = EXTI_PR1_PIF5;
        if (render) {
            return;
        }
        if (state == STATE_FLASHCARD_NAVIGATION) {
            f_b = !f_b;
        } else {
            state = STATE_FLASHCARD_NAVIGATION;
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

int main(void)
{
    spi1_init();

    eink_init();

    button_init();

    setup_deck();

    // #define WRAP_TEST
    #ifdef WRAP_TEST
    eink_clear(0xFF);
    char str[150] = "Hello, this is a test of the wrapping on the display :) lkasdflaskldfj kasdkjl asdflk jj mm fjfjjf fjjfjfjf jjfjf";
    // char str[100] = "Hello :)";
    draw_string_wrapped(0, 0, str, BLACK);
    draw_string(0, 35, str, BLACK);
    eink_render_framebuffer();
    #endif /* WRAP_TEST */

    // #define FLASHCARD_FRONT
    #ifdef FLASHCARD_FRONT
    eink_clear(0xFF);
    struct flashcard fc;
    char front[250] = "MITOCHONDRIA";
    // char back[300] = "It is the powerhouse of the cell. This is learned in Biology class.";
    char back[300] = "Biology class. asdflk jlkasjdf alksdjf kljasdg f";
    strcpy(fc.front, front);
    strcpy(fc.back, back);
    draw_flashcard(fc, 1, BLACK);
    eink_render_framebuffer();
    #endif /* FLASHCARD_FRONT */

    // #define CLEAR
    #ifdef CLEAR
    eink_clear(0xFF);
    eink_render_framebuffer();
    #endif /* CLEAR */

    #define STATE_MACHINE
    #ifdef STATE_MACHINE
    eink_clear(0xFF);
    draw_main_menu(0, deck_names, 6);
    eink_render_framebuffer();
    state_machine();
    #endif /* STATE_MACHINE */

    // #define SHIFT
    #ifdef SHIFT
    for (uint8_t i = 0; i < 6; ++i) {
        eink_clear(0xFF);

        draw_main_menu(i);

        eink_render_framebuffer();
    }
    #endif /* STATE */

    eink_sleep();

    for (;;);
    return 0;
}
