#ifndef SPI_H
#define SPI_H

#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

// void spi1_init();
// void spi_tx(SPI_TypeDef* spi, uint8_t data);
// void spi_rx(SPI_TypeDef* spi, uint8_t data);
// void spi_tx_buffer(SPI_TypeDef* spi, uint8_t* data, size_t size);
// void spi_rx_buffer(SPI_TypeDef* spi, uint8_t* data, size_t size);

void init_spi1_slow();
void enable_sdcard();
void disable_sdcard();
void init_sdcard_io();
void sdcard_io_high_speed();



#endif /* SPI_H */
