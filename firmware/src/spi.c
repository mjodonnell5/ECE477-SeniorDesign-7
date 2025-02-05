#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

/* For e-ink */
void spi1_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* NSS = PA4, SCK = PA5, MOSI = PA7 */

    GPIOA->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER7);
    GPIOA->MODER |= (GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1);

    /* AF5 */
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL7);
    GPIOA->AFR[0] |= (GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL4_2 
                    | GPIO_AFRL_AFSEL5_0 | GPIO_AFRL_AFSEL5_2
                    | GPIO_AFRL_AFSEL7_0 | GPIO_AFRL_AFSEL7_2);

    SPI1->CR1 &= ~(SPI_CR1_SPE);

    /* Divide by 16 (We have 32Mhz clock) */
    SPI1->CR1 &= ~SPI_CR1_BR;
    SPI1->CR1 |= (SPI_CR1_BR_1 | SPI_CR1_BR_0);

    /* 8 bit word size */
    SPI1->CR2 &= SPI_CR2_DS;
    SPI1->CR2 |= (SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0);

    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR2 |= SPI_CR2_SSOE;
    SPI1->CR2 |= SPI_CR2_NSSP;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_tx(SPI_TypeDef* spi, uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE));

    SPI1->DR = data;

    while (SPI1->DR & SPI_SR_BSY);
}

void spi_rx(SPI_TypeDef* spi, uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_RXNE));

    data = SPI1->DR;
}

void spi_tx_buffer(SPI_TypeDef* spi, uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; ++size) {
        while (!(SPI1->SR & SPI_SR_TXE));

        SPI1->DR = data[i];

        while (SPI1->DR & SPI_SR_BSY);
    }
}

void spi_rx_buffer(SPI_TypeDef* spi, uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; ++size) {
        while (!(SPI1->SR & SPI_SR_RXNE));

        data[i] = SPI1->DR;
    }
}
