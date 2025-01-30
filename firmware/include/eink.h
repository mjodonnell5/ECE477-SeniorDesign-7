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
void eink_draw_hline(uint16_t x, uint16_t y, uint8_t width, uint8_t c);
void eink_sleep();
void eink_send_data(uint8_t data);
void eink_send_cmd(uint8_t cmd);

#endif /* EINK_H */
