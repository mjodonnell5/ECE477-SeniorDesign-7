#ifndef UI_H
#define UI_H

#include <stdint.h>

#include "../include/font.h"

#define FRONT (1)
#define BACK  (0)
#define MAX_FRONT_SIZE     (100)
#define MAX_BACK_SIZE      (200)
#define MAX_NAME_SIZE      (25)
#define MAX_CARDS_PER_DECK (20)
#define MAX_DECKS          (10)

#define MAX_ITEMS_PER_PAGE (6)

struct flashcard {
    char front[MAX_FRONT_SIZE];
    char back[MAX_BACK_SIZE];
};

struct deck {
    char name[MAX_NAME_SIZE];
    uint8_t num_cards;
    struct flashcard cards[MAX_CARDS_PER_DECK];
};

void parse_metadata(uint8_t num_decks, char deck_names[][MAX_NAME_SIZE]);

void draw_sleep_image(uint16_t s_x, uint16_t s_y);

void draw_menu(uint8_t curr_selected, char names[][MAX_NAME_SIZE], uint16_t num);
void draw_main_menu(uint8_t curr_selected_deck, char deck_names[][MAX_NAME_SIZE], uint16_t num_decks);
void draw_hline(uint16_t x, uint16_t y, uint16_t length, uint8_t col);
void draw_vline(uint16_t x, uint16_t y, uint16_t length, uint8_t col);
void draw_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col);
void draw_string(struct font f, uint16_t s_x, uint16_t s_y, char* string, uint8_t col);
void draw_char(struct font f, uint16_t s_x, uint16_t s_y, uint16_t c, uint8_t col);
void draw_centered_string_in_rect(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col);
void draw_centered_string_wrapped(struct font f, char* string, uint8_t col);
void draw_centered_string_in_filled_rect(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col);
void draw_filled_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col);
void draw_flashcard(struct flashcard fc, uint8_t f_b, uint8_t col);
void draw_string_wrapped(struct font f, uint16_t s_x, uint16_t s_y, char* string, uint8_t col);
void draw_header(char* title);

void draw_filled_deck_menu_item(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, char* string2, uint8_t col);
void draw_deck_menu_item(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, char* string2, uint8_t col);

#endif /* UI_H */
