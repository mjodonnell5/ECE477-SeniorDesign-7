#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32l432xx.h>

#include "../include/eink.h"
#include "../include/clock.h"
#include "../include/ui.h"

#define DIST_DECKS (45)

extern uint8_t font8x8_basic[128][8];
extern struct deck main_deck;

void draw_header(char* title)
{
    /* Current title */
    draw_centered_string_in_filled_rect(0, 0, EINK_WIDTH - 1, 10, title, BLACK);

    /* Battery percentage */
    char battery_perc_str[9];
    uint8_t battery_perc = 100;
    snprintf(battery_perc_str, 9, "BAT:%d%%", battery_perc);
    draw_string(EINK_WIDTH - (8 * strlen(battery_perc_str)), 2, battery_perc_str, WHITE);
}

void draw_main_menu(uint8_t curr_selected_deck, char deck_names[][MAX_NAME_SIZE], uint16_t num_decks)
{
    uint8_t curr_page = curr_selected_deck / MAX_DECKS_PER_PAGE;
    uint8_t decks_on_page = num_decks - (curr_page * MAX_DECKS_PER_PAGE);

    /* If curr_page == 0 */
    if (decks_on_page > MAX_DECKS_PER_PAGE) {
        decks_on_page = MAX_DECKS_PER_PAGE;
    }

    for (uint8_t i = 0; i < decks_on_page; ++i) {
        uint8_t index = i + (curr_page * MAX_DECKS_PER_PAGE);
        if (index == curr_selected_deck) {
            /* Draw filled & inverted rectangle */
            draw_centered_string_in_filled_rect(0 + 50, 20 + DIST_DECKS * i, EINK_WIDTH - 1 - 50, 60 + DIST_DECKS * i, deck_names[index], BLACK);
        } else {
            /* Normal */
            draw_centered_string_in_rect(0 + 50, 20 + DIST_DECKS * i, EINK_WIDTH - 1 - 50, 60 + DIST_DECKS * i, deck_names[index], BLACK);
        }
    }
}

void draw_flashcard(struct flashcard fc, uint8_t f_b, uint8_t col)
{
    if (f_b) {
        // draw_header("FLASHCARD FRONT");
        // draw_string_wrapped(20, 20, fc.front, col);
        draw_centered_string_wrapped(fc.front, col);
    } else {
        // draw_header("FLASHCARD BACK");
        draw_centered_string_wrapped(fc.back, col);
        // draw_string_wrapped(20, 20, fc.back, col);
    }
}

void draw_hline(uint16_t x, uint16_t y, uint16_t length, uint8_t col)
{
    for (uint16_t i = x; i < x + length; ++i) {
        eink_draw_pixel(i, y, col);
    }
}

void draw_vline(uint16_t x, uint16_t y, uint16_t length, uint8_t col)
{
    for (uint16_t i = y; i < y + length; ++i) {
        eink_draw_pixel(x, i, col);
    }
}

void draw_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col)
{
    uint16_t min_x, min_y, max_x, max_y;
    min_x = e_x > s_x ? s_x : e_x;
    max_x = e_x > s_x ? e_x : s_x;
    min_y = e_y > s_y ? s_y : e_y;
    max_y = e_y > s_y ? e_y : s_y;
    
    draw_hline(min_x, min_y, max_x - min_x + 1, col);
    draw_hline(min_x, max_y, max_x - min_x + 1, col);
    draw_vline(min_x, min_y, max_y - min_y + 1, col);
    draw_vline(max_x, min_y, max_y - min_y + 1, col);
}

void draw_filled_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, uint8_t col)
{
    uint16_t min_x, min_y, max_x, max_y;
    min_x = e_x > s_x ? s_x : e_x;
    max_x = e_x > s_x ? e_x : s_x;
    min_y = e_y > s_y ? s_y : e_y;
    max_y = e_y > s_y ? e_y : s_y;
    
    draw_hline(min_x, min_y, max_x - min_x + 1, col);
    draw_hline(min_x, max_y, max_x - min_x + 1, col);
    draw_vline(min_x, min_y, max_y - min_y + 1, col);
    draw_vline(max_x, min_y, max_y - min_y + 1, col);

    for (uint16_t y = min_y; y <= max_y; ++y) {
        draw_hline(min_x, y, max_x - min_x, col);
    }
}

void draw_char(uint16_t s_x, uint16_t s_y, uint16_t c, uint8_t col)
{ 
    const uint8_t* bitmap = font8x8_basic[c];

    for (uint8_t y = 0; y < 8; ++y) {
        for (uint8_t x = 0; x < 8; ++x) {
            uint8_t is_set = bitmap[y] & (1 << x);
            if (is_set) {
                eink_draw_pixel(s_x + x, s_y + y, col);
            }
        }
    }
}

void draw_string(uint16_t s_x, uint16_t s_y, char* string, uint8_t col)
{
    while (*string) {
        draw_char(s_x, s_y, *string, col);

        string++;
        s_x += 8;
    }
}

void draw_string_wrapped(uint16_t s_x, uint16_t s_y, char* string, uint8_t col) {
    uint16_t c_x = s_x;
    uint16_t c_y = s_y;

    char buffer[2048];
    strncpy(buffer, string, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* word = strtok(buffer, " ");

    while (word != NULL) {
        uint16_t word_length_pixels = strlen(word) * 8;

        /* subtracting c_x will "center" it so that it wraps not at the end
         * of the display, but so that the text is centered. */
        /* FIXME: Should I have a mode for this center wrapping or have it
         * be default? */
        if (word_length_pixels + c_x >= EINK_WIDTH - s_x) {
            /* Write this word on next line */
            c_x = s_x;
            c_y += 10;
        }

        draw_string(c_x, c_y, word, col);

        /* +8 is to add a space after the word*/
        c_x += word_length_pixels + 8;

        word = strtok(NULL, " ");
    }
}

void draw_centered_string_in_filled_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col)
{
    uint16_t min_x = e_x > s_x ? s_x : e_x;
    uint16_t max_x = e_x > s_x ? e_x : s_x;
    uint16_t min_y = e_y > s_y ? s_y : e_y;
    uint16_t max_y = e_y > s_y ? e_y : s_y;

    uint16_t width = max_x - min_x + 1;
    uint16_t height = max_y - min_y + 1;
    
    uint16_t text_width = strlen(string) * 8;
    uint16_t text_height = 8;

    uint16_t text_x = min_x + (width - text_width) / 2;
    uint16_t text_y = min_y + (height - text_height) / 2;

    draw_filled_rect(min_x, min_y, max_x, max_y, col);

    draw_string(text_x, text_y, string, !col);
}

void draw_centered_string_in_rect(uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col)
{
    uint16_t min_x = e_x > s_x ? s_x : e_x;
    uint16_t max_x = e_x > s_x ? e_x : s_x;
    uint16_t min_y = e_y > s_y ? s_y : e_y;
    uint16_t max_y = e_y > s_y ? e_y : s_y;

    uint16_t width = max_x - min_x + 1;
    uint16_t height = max_y - min_y + 1;
    
    uint16_t text_width = strlen(string) * 8;
    uint16_t text_height = 8;

    uint16_t text_x = min_x + (width - text_width) / 2;
    uint16_t text_y = min_y + (height - text_height) / 2;

    draw_rect(min_x, min_y, max_x, max_y, col);

    draw_string(text_x, text_y, string, col);
}

void draw_centered_string_wrapped(char* string, uint8_t col)
{
    uint16_t text_width = strlen(string) * 8;
    uint8_t num_wraps = text_width / (EINK_WIDTH - 20);
    uint8_t height = (EINK_HEIGHT - (num_wraps * 10)) / 2;

    uint8_t width = (EINK_WIDTH - text_width) / 2;
    if (num_wraps > 0) width = 20;

    draw_string_wrapped(width, height, string, col);
}
