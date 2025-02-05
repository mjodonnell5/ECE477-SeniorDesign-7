#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../include/spi.h"
#include "../include/eink.h"
#include "../include/clock.h"

uint8_t framebuffer[EINK_FRAMEBUFFER_SIZE] = {WHITE};

static void eink_send_cmd(uint8_t cmd)
{
    /* D/C low */
    GPIOA->BSRR = GPIO_BSRR_BR8;

    /* CS low */
    GPIOA->BSRR = GPIO_BSRR_BR4;

    spi_tx(EINK_SPI, cmd);

    /* CS high */
    GPIOA->BSRR = GPIO_BSRR_BS4;
}

static void eink_send_data(uint8_t data)
{
    /* D/C high */
    GPIOA->BSRR = GPIO_BSRR_BS8;

    /* CS low */
    GPIOA->BSRR = GPIO_BSRR_BR4;

    spi_tx(EINK_SPI, data);

    /* CS high */
    GPIOA->BSRR = GPIO_BSRR_BS4;
}

static void eink_pin_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* BUSY - PA6 (INPUT) ; RESET - PA9 ; D/C - PA8 */
    GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
    GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

    /* High speed */
    GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED8 | GPIO_OSPEEDR_OSPEED9);
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_1 | GPIO_OSPEEDR_OSPEED8_1 | GPIO_OSPEEDR_OSPEED9_1;

}

static void eink_wait_until_idle()
{
    /* The all mightly magical command */
    eink_send_cmd(0x71);

    while((GPIOA->IDR & GPIO_IDR_ID6) == 1) {
        eink_send_cmd(0x71);
    }

    /* FIXME: Does it need to be this high?? */
    delay_ms(200);
}

static void eink_reset()
{
    /* Set RESET high */
    GPIOA->BSRR = GPIO_BSRR_BS9;
    delay_ms(100);

    /* RESET is active low */
    GPIOA->BSRR = GPIO_BSRR_BR9;

    delay_ms(2);

    /* Set RESET high */
    GPIOA->BSRR = GPIO_BSRR_BS9;

    delay_ms(100);
}

void eink_init()
{
    eink_pin_init();

    eink_reset();
    eink_send_cmd(0x12);
    eink_wait_until_idle();

    delay_ms(1);

    eink_send_cmd(0x21);
    eink_send_data(0x40);
    eink_send_data(0x00);

    eink_send_cmd(0x3C);
    eink_send_data(0x05);

    eink_send_cmd(0x11);
    eink_send_data(0x03);

    eink_send_cmd(0x44); 
    eink_send_data(0x00);
    eink_send_data(0x31); 
    
    eink_send_cmd(0x45);
    eink_send_data(0x00);
    eink_send_data(0x00);  
    eink_send_data(0x2B);
    eink_send_data(0x01);

    eink_send_cmd(0x4E); 
    eink_send_data(0x00);

    eink_send_cmd(0x4F); 
    eink_send_data(0x00);
    eink_send_data(0x00);  
    eink_wait_until_idle();
}

static inline void eink_draw_pixel(uint16_t x, uint16_t y, uint8_t col)
{
    uint16_t index = y * EINK_WIDTH + x;
    uint16_t byte_index = index / 8;

    if (x >= EINK_WIDTH || y >= EINK_HEIGHT) return;

    if (col == WHITE) {
        framebuffer[byte_index] |= (0x80 >> (x % 8));
    } else {
        framebuffer[byte_index] &= ~(0x80 >> (x % 8));
    }
}

void eink_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint8_t col)
{
    for (uint16_t i = x; i < x + length; ++i) {
        eink_draw_pixel(i, y, col);
    }
}

void eink_draw_vline(uint16_t x, uint16_t y, uint16_t length, uint8_t col)
{
    for (uint16_t i = y; i < y + length; ++i) {
        eink_draw_pixel(x, i, col);
    }
}

void eink_draw_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col)
{
    uint16_t length = e_x - s_x;
    uint16_t height = e_y - s_y;
    eink_draw_hline(s_x, s_y, length, col);
    eink_draw_hline(s_x, s_y - height, height, col);
    eink_draw_vline(s_x, s_y, height, col);
    eink_draw_vline(e_x, e_y, height, col);
}

// void eink_draw_char(uint16_t x, uint16_t y, uint16_t c, uint8_t col)
// {
// }

static void eink_turn_on_display()
{
    /* Update display */
    eink_send_cmd(0x22);
    eink_send_data(0xF7);
    eink_send_cmd(0x20);
    eink_wait_until_idle();
}

void eink_render_framebuffer()
{
    /* Start writing into B/W ram */
    eink_send_cmd(0x24);

    for (uint16_t i = 0; i < EINK_FRAMEBUFFER_SIZE; ++i) {
        eink_send_data(framebuffer[i]);
    }

    eink_wait_until_idle();
    eink_turn_on_display();
}

void eink_clear()
{
    memset(framebuffer, WHITE, EINK_FRAMEBUFFER_SIZE);
}

void eink_sleep()
{
    eink_send_cmd(0x10);
    eink_send_data(0x01);
    delay_ms(100);
}
