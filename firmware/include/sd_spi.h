#ifndef SPI_H
#define SPI_H

#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

void init_spi1_slow();
void enable_sdcard();
void disable_sdcard();
void init_sdcard_io();
void sdcard_io_high_speed();



#endif /* SPI_H */
