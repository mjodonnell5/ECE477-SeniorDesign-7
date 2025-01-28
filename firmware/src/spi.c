#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

/* For e-ink */
void spi1_init()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER7 | GPIO_MODER_MODER15);
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER15_1);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL7);
    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL15);

    SPI1->CR1 &= ~(SPI_CR1_SPE);
    SPI1->CR1 |= SPI_CR1_BR; /* Lowest baud rate */
    SPI1->CR2 &= ~(SPI_CR2_DS_2 | SPI_CR2_DS_1); /* 10 bit word size */
    SPI1->CR2 |= SPI_CR2_DS_3 | SPI_CR2_DS_0; /* 10 bit word size (0b1001) */
    SPI1->CR1 |= SPI_CR1_MSTR; /* Set to master */
    SPI1->CR2 |= SPI_CR2_SSOE;
    SPI1->CR2 |= SPI_CR2_NSSP;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void spi_tx(SPI_TypeDef* spi, uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; ++size) {
        while (!(SPI1->SR & SPI_SR_TXE));

        SPI1->DR = data[i];

        while (SPI1->DR & SPI_SR_BSY);
    }
}

void spi_rx(SPI_TypeDef* spi, uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; ++size) {
        while (!(SPI1->SR & SPI_SR_RXNE));

        data[i] = SPI1->DR;
    }
}
