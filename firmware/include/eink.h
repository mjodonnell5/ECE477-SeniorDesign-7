#ifndef EINK_H
#define EINK_H

#include <stm32l496xx.h>
#include <stdint.h>
#include <stddef.h>

#define EINK_SPI (SPI1)

#define EINK_HEIGHT (300)
#define EINK_WIDTH  (400)

#define EINK_FRAMEBUFFER_SIZE ((EINK_WIDTH * EINK_HEIGHT) / 8)

#define WHITE (1)
#define BLACK (0)

void eink_init();
void eink_sleep();
void eink_render_framebuffer();
void eink_clear(uint8_t col);
extern void eink_draw_pixel(uint16_t x, uint16_t y, uint8_t col);
void eink_wait_until_idle();

#endif /* EINK_H */
