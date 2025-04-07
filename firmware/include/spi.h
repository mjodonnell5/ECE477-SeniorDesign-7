#ifndef SPI_H
#define SPI_H

#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

void spi1_init();
void spi_tx(SPI_TypeDef* spi, uint8_t data);
void spi_rx(SPI_TypeDef* spi, uint8_t* data);
void spi_tx_buffer(SPI_TypeDef* spi, uint8_t* data, size_t size);
void spi_rx_buffer(SPI_TypeDef* spi, uint8_t* data, size_t size);

#endif /* SPI_H */
