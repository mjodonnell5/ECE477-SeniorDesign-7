#ifndef UI_H
#define UI_H

#include <stdint.h>

struct flashcard {
    char front[250];
    char back[300];
};

void draw_main_menu(uint8_t curr_selected_deck);
void draw_hline(uint16_t x, uint16_t y, uint16_t length, uint8_t col);
void draw_vline(uint16_t x, uint16_t y, uint16_t length, uint8_t col);
void draw_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col);
void draw_string(uint16_t s_x, uint16_t s_y, char* string, uint8_t col);
void draw_char(uint16_t s_x, uint16_t s_y, uint16_t c, uint8_t col);
void draw_centered_string_in_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col);
void draw_centered_string_in_filled_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col);
void draw_filled_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col);
void draw_flashcard(struct flashcard fc, uint8_t f_b, uint8_t col);
void draw_string_wrapped(uint16_t s_x, uint16_t s_y, char* string, uint8_t col);

#endif /* UI_H */
