#ifndef UI_H
#define UI_H

#include <stdint.h>

#define FRONT (1)
#define BACK (0)
#define MAX_FRONT_SIZE     (100)
#define MAX_BACK_SIZE      (200)
#define MAX_CARDS_PER_DECK (5)

struct flashcard {
    /* Metadata */
    /* ... */

    char front[MAX_FRONT_SIZE];
    char back[MAX_BACK_SIZE];
};

struct deck {
    /* Metadata */
    /* ... */

    struct flashcard cards[MAX_CARDS_PER_DECK];
};

void draw_main_menu(uint8_t curr_selected_deck, char* deck_names[], uint16_t num_decks);
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
void draw_header(char* title);

#endif /* UI_H */
