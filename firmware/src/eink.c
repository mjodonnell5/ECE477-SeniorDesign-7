#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#include "../include/spi.h"
#include "../include/eink.h"
#include "../include/clock.h"

uint8_t framebuffer[(EINK_WIDTH * EINK_HEIGHT) / 8];

void eink_send_cmd(uint8_t cmd)
{
    /* CS Low */
    GPIOA->ODR &= ~(GPIO_ODR_OD4);

    GPIOA->ODR &= ~GPIO_ODR_OD8;
    spi_tx(EINK_SPI, cmd);

    GPIOA->ODR |= GPIO_ODR_OD4;
}

void eink_send_data(uint8_t data)
{
    GPIOA->ODR &= ~(GPIO_ODR_OD4);

    GPIOA->ODR |= GPIO_ODR_OD8;
    spi_tx(EINK_SPI, data);

    GPIOA->ODR |= GPIO_ODR_OD4;
}

static void eink_pin_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    /* BUSY - PA6 (INPUT) ; RESET - PA9 ; D/C - PA8 */
    GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER8 | GPIO_MODER_MODER9);
    GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

    /* Set RESET high */
    GPIOA->ODR |= GPIO_ODR_OD9;
}

static void eink_wait_until_idle()
{
    /* BUSY is active low, return when it goes high */
    while(!(GPIOA->IDR & GPIO_IDR_ID6));
}

static void eink_reset()
{
    /* RESET is active low */
    GPIOA->ODR &= ~GPIO_ODR_OD9;

    delay_ms(300);

    GPIOA->ODR |= GPIO_ODR_OD9;

    delay_ms(300);
    // delay_us(200);
}

void eink_init()
{
    eink_pin_init();

    /* Wait 10ms after power on */
    delay_ms(10);

    /* Hardware reset */
    eink_reset();
    eink_wait_until_idle();

    /* Software reset */
    eink_send_cmd(0x12);
    eink_wait_until_idle();

    /* Display size */
    eink_send_cmd(0x01);
    eink_send_data(0x2B);
    eink_send_data(0x01);
    eink_send_data(0x00);

    /* Ram data entry mode (automation increment) */
    eink_send_cmd(0x11);
    eink_send_data(0x03);

    /* Display update control (Ignore red pixels) */
    eink_send_cmd(0x21);
    eink_send_data(0x40);

    /* Set Ram X address */
    eink_send_cmd(0x44);
    eink_send_data(0x00); /* Start */
    eink_send_data(0x31); /* End */

    /* Set Ram Y address */
    eink_send_cmd(0x45);
    eink_send_data(0x00); /* Start LSB */
    eink_send_data(0x00); /* Start MSB */
    eink_send_data(0x2B); /* End LSB */
    eink_send_data(0x01); /* End MSB */

    /* Set Ram X address counter */
    eink_send_cmd(0x4E);
    eink_send_data(0x00);

    /* Set Ram Y address counter */
    eink_send_cmd(0x4F);
    eink_send_data(0x00);
    eink_send_data(0x00);
}

void eink_draw_pixel(uint16_t x, uint16_t y, uint8_t c)
{
    uint16_t index = y * EINK_WIDTH + x;

    if (c) {
        framebuffer[index / 8] |= (0x80 >> (x % 8));
    } else {
        framebuffer[index / 8] &= ~(0x80 >> (x % 8));
    }
}

void eink_draw_hline(uint16_t x, uint16_t y, uint8_t width, uint8_t c)
{
    for (uint16_t i = x; i < x + width; ++i) {
        eink_draw_pixel(i, y, c);
    }
}

void eink_render_framebuffer()
{
    /* Start writing into B/W ram */
    eink_send_cmd(0x24);
    for (uint16_t x = 0; x < EINK_WIDTH; ++x) {
        for (uint16_t y = 0; y < EINK_HEIGHT; ++y) {
            uint16_t index = y * EINK_WIDTH + x;
            eink_send_data(framebuffer[index]);
        }
    }

    /* Soft start? */
    // eink_send_cmd(0x0C);

    /* Update display */
    eink_send_cmd(0x22);
    eink_send_data(0xC7);
    eink_send_cmd(0x20);
}

void eink_sleep()
{
    eink_send_cmd(0x10);
    eink_send_data(0x01);
}
