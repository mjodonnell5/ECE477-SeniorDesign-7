#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#include "../include/spi.h"
#include "../include/eink.h"
#include "../include/clock.h"

uint8_t framebuffer[EINK_FRAMEBUFFER_SIZE] = {0xFF};

void eink_send_cmd(uint8_t cmd)
{
    /* D/C low */
    GPIOA->BSRR = GPIO_BSRR_BR8;

    /* CS low */
    GPIOA->BSRR = GPIO_BSRR_BR4;

    spi_tx(EINK_SPI, cmd);

    /* CS high */
    GPIOA->BSRR = GPIO_BSRR_BS4;
}

void eink_send_data(uint8_t data)
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

void eink_draw_pixel(uint16_t x, uint16_t y, uint8_t c)
{
    uint16_t index = y * EINK_WIDTH + x;

    if (c) {
        framebuffer[index / 8] |= (0x80 >> (x % 8));
    } else {
        framebuffer[index / 8] &= ~(0x80 >> (x % 8));
    }
}

void eink_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint8_t c)
{
    for (uint16_t i = x; i < x + length; ++i) {
        eink_draw_pixel(i, y, c);
    }
}

void eink_turn_on_display()
{
    /* Update display */
    eink_send_cmd(0x22);
    eink_send_data(0xF7);
    eink_send_cmd(0x20);
    eink_wait_until_idle();
}

void eink_render_framebuffer()
{
    uint16_t width;
    uint16_t height;
	width = EINK_WIDTH / 8;
	height = EINK_HEIGHT;

    /* Start writing into B/W ram */
    eink_send_cmd(0x24);
    delay_ms(1);
    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            uint16_t index = y * width + x;
            eink_send_data(framebuffer[index]);
        }
    }

    delay_ms(10);

    eink_wait_until_idle();
    eink_turn_on_display();
}

void eink_clear()
{
    uint16_t width;
    uint16_t height;
	width = EINK_WIDTH / 8;
	height = EINK_HEIGHT;

	eink_send_cmd(0x24);
    delay_ms(1);
	for (uint16_t j = 0; j < height; j++) {
        for (uint16_t i = 0; i < width; i++) {
            eink_send_data(0xFF);
        }
	}

    // for (uint16_t k = 0; k < 15000; k++) {
    //     eink_send_data(0xFF);
    // }

 //    eink_send_cmd(0x26);
 //    for (unsigned int j = 0; j < height; j++) {
 //        for (unsigned int i = 0; i < width; i++) {
 //            eink_send_data(0xFF);
 //        }
	// }
    delay_ms(10);

    eink_wait_until_idle();
	eink_turn_on_display();
}

void eink_sleep()
{
    eink_send_cmd(0x10);
    eink_send_data(0x01);
    delay_ms(100);
}
