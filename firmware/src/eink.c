#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stm32l496xx.h>

#include "../include/spi.h"
#include "../include/eink.h"
#include "../include/clock.h"
#include "../include/state.h"

uint8_t framebuffer[EINK_FRAMEBUFFER_SIZE] = {0xFF};
volatile uint8_t eink_busy = 0;

static void eink_send_cmd(uint8_t cmd)
{
    /* D/C low */
    GPIOC->BSRR = GPIO_BSRR_BR4;

    /* CS low */
    GPIOA->BSRR = GPIO_BSRR_BR4;

    spi_tx(EINK_SPI, cmd);

    /* CS high */
    GPIOA->BSRR = GPIO_BSRR_BS4;
}

static void eink_send_data(uint8_t data)
{
    /* D/C high */
    GPIOC->BSRR = GPIO_BSRR_BS4;

    /* CS low */
    GPIOA->BSRR = GPIO_BSRR_BR4;

    spi_tx(EINK_SPI, data);

    /* CS high */
    GPIOA->BSRR = GPIO_BSRR_BS4;
}

static void eink_pin_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; //turn on A
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN; //turn on C

    /* BUSY - PA6 (INPUT) ; RESET - PA9 ; D/C - PA8 */
    GPIOA->MODER &= ~(GPIO_MODER_MODER6 );
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PA;
    EXTI->FTSR1 |= EXTI_FTSR1_FT6;
    EXTI->IMR1 |= EXTI_IMR1_IM6;

    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);
    // GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

    /*; RESET - PC5 ; D/C - PC4 */
    GPIOC->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5);
    GPIOC->MODER |= (GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0);

    /* High speed ...might need these?*/
    // GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED8 | GPIO_OSPEEDR_OSPEED9);
    // GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_1 | GPIO_OSPEEDR_OSPEED8_1 | GPIO_OSPEEDR_OSPEED9_1;

}


void eink_wait_until_idle()
{
    /* The all mightly magical command */
    // eink_send_cmd(0x71);

    // while((GPIOA->IDR & GPIO_IDR_ID6) == 1) {
    //     eink_send_cmd(0x71);
    // }
    while (eink_busy == 1) {};

    // delay_ms(100);
}

static void eink_reset()
{
    /* Set RESET high */
    GPIOC->BSRR = GPIO_BSRR_BS5;
    delay_ms(100);

    /* RESET is active low */
    GPIOC->BSRR = GPIO_BSRR_BR5;

    delay_ms(2);

    /* Set RESET high */
    GPIOC->BSRR = GPIO_BSRR_BS5;

    delay_ms(100);
}

void eink_init()
{
    eink_pin_init();

    eink_reset();
    eink_send_cmd(0x12);
    eink_wait_until_idle();
    delay_ms(100);

    delay_ms(10);

    eink_send_cmd(0x21);
    eink_send_data(0x40);
    eink_send_data(0x00);

    eink_send_cmd(0x3C);
    eink_send_data(0x05);

    /* Stuff for FAST MODE */
    eink_send_cmd(0x1A);
    eink_send_data(0x5A);
    eink_send_cmd(0x22);
    eink_send_data(0x91);
    eink_send_cmd(0x20);
    eink_wait_until_idle();
    delay_ms(100);

    eink_send_cmd(0x11);
    if (left_handed) {
        eink_send_data(0x03);
    } else {
        eink_send_data(0x00);
    }

    eink_send_cmd(0x44); 
    if (left_handed) {
        eink_send_data(0x00);
        eink_send_data(0x31); 
    } else {
        eink_send_data(0x31); 
        eink_send_data(0x00);
    }
    
    eink_send_cmd(0x45);
    if (left_handed) {
        eink_send_data(0x00);
        eink_send_data(0x00);  
        eink_send_data(0x2B);
        eink_send_data(0x01);
    } else {
        eink_send_data(0x2B);
        eink_send_data(0x01);
        eink_send_data(0x00);
        eink_send_data(0x00);  
    }

    if (left_handed) {
        eink_send_cmd(0x4E); 
        eink_send_data(0x00);

        eink_send_cmd(0x4F); 
        eink_send_data(0x00);
        eink_send_data(0x00);  
    } else {
        eink_send_cmd(0x4E); 
        eink_send_data(0x31);

        eink_send_cmd(0x4F); 
        eink_send_data(0x2B);
        eink_send_data(0x01);  
    }
    eink_wait_until_idle();
    // delay_ms(100);
}

inline void eink_draw_pixel(uint16_t x, uint16_t y, uint8_t col)
{
    uint32_t index = y * EINK_WIDTH + x;
    uint16_t byte_index = index / 8;

    if (x >= EINK_WIDTH || y >= EINK_HEIGHT) return;

    if (col == WHITE) {
        framebuffer[byte_index] |= (0x80 >> (x % 8));
    } else {
        framebuffer[byte_index] &= ~(0x80 >> (x % 8));
    }
}

static void eink_turn_on_display()
{
    /* FAST MODE */
    eink_send_cmd(0x22);
    eink_send_data(0xC7);
    eink_send_cmd(0x20);
    eink_wait_until_idle();
}

void eink_render_framebuffer()
{
    eink_busy = 1;
    /* Start writing into B/W ram */
    eink_send_cmd(0x24);

    for (uint16_t i = 0; i < EINK_FRAMEBUFFER_SIZE; ++i) {
        eink_send_data(framebuffer[i]);
    }

    eink_turn_on_display();
}

void eink_clear(uint8_t col)
{
    memset(framebuffer, col, EINK_FRAMEBUFFER_SIZE);
}

void eink_sleep()
{
    eink_send_cmd(0x10);
    eink_send_data(0x01);
    delay_ms(100);
}
