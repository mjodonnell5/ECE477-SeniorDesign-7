#ifndef EINK_H
#define EINK_H

#include <stm32l432xx.h>
#include <stdint.h>
#include <stddef.h>

#define EINK_SPI (SPI1)

#define EINK_HEIGHT (300)
#define EINK_WIDTH  (400)

#define EINK_FRAMEBUFFER_SIZE ((EINK_WIDTH * EINK_HEIGHT) / 8)

#define WHITE (0xFF)
#define BLACK (0x00)

void eink_init();
void eink_draw_hline(uint16_t x, uint16_t y, uint16_t length, uint8_t col);
void eink_draw_vline(uint16_t x, uint16_t y, uint16_t length, uint8_t col);
void eink_draw_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col);
void eink_sleep();
static void eink_send_data(uint8_t data);
static void eink_send_cmd(uint8_t cmd);
void eink_render_framebuffer();
void eink_clear();
static void eink_turn_on_display();
static inline void eink_draw_pixel(uint16_t x, uint16_t y, uint8_t col);
void eink_draw_string(uint16_t s_x, uint16_t s_y, char* string, uint8_t col);
void eink_draw_char(uint16_t s_x, uint16_t s_y, uint16_t c, uint8_t col);

/* TESTING ONLY */
void draw_pixel(uint16_t x, uint16_t y, uint8_t col);

#endif /* EINK_H */
