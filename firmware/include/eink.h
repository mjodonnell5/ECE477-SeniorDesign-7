#ifndef EINK_H
#define EINK_H

#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#define EINK_SPI (SPI1)

void eink_init();

#endif /* EINK_H */
