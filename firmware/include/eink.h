#ifndef EINK_H
#define EINK_H

#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#define EINK_SPI (SPI1)

#define EINK_HEIGHT (300)
#define EINK_WIDTH  (400)

#define WHITE_PIXEL (1)
#define BLACK_PIXEL (0)

void eink_init();

#endif /* EINK_H */
