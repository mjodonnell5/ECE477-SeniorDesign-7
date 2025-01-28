#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#include "../include/spi.h"
#include "../include/eink.h"

static void eink_send_cmd(uint8_t cmd)
{
    spi_tx(EINK_SPI, cmd);
}

static void eink_send_data(uint8_t data)
{
    spi_tx(EINK_SPI, data);
}

static void eink_pin_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    /* BUSY - PA6 */
    /* RESET - PA7 */
    /* D/C - PA8 */
    GPIOA->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8);
    GPIOA->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0);
}

static void eink_wait_until_idle()
{
    /* BUSY is active low, return when it goes high */
    while(!(GPIOA->IDR & GPIO_IDR_ID6));
}

static void eink_reset()
{
    /* RESET is active low */
    GPIOA->ODR &= ~GPIO_ODR_OD7;

    /* WAIT */

    GPIOA->ODR |= GPIO_ODR_OD7;
}

void eink_init()
{
    /* Reset */
    eink_reset();

    /* Booster soft start */
    eink_send_cmd(0x06);
    eink_send_data(0x17);
    eink_send_data(0x17);
    eink_send_data(0x17);

    /* Power setting */
    eink_send_cmd(0x01);
    eink_send_data(0x03);
    eink_send_data(0x00);
    eink_send_data(0x2b);
    eink_send_data(0x2b);
    eink_send_data(0x09);

    /* Power on */
    eink_send_cmd(0x04);

    eink_wait_until_idle();

    /* Panel setting */
    eink_send_cmd(0x00);
    eink_send_data(0xbf);

    /* PLL control */
    eink_send_cmd(0x61);
    eink_send_data(0x01);
    eink_send_data(0x90);
    eink_send_data(0x01);
    eink_send_data(0x2c);

    /* VCM_DC setting */
    eink_send_cmd(0x82);
    eink_send_data(0x12);

    /* Vcom and data interval setting */
    eink_send_cmd(0x50);
    eink_send_data(0x87);
}
