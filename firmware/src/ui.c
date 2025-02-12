#include <stm32l432xx.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "../include/eink.h"
#include "../include/clock.h"
#include "../include/ui.h"

#define DIST_DECKS (45)

extern uint8_t font8x8_basic[128][8];

void draw_main_menu(uint8_t curr_selected_deck)
{
    /* Draw header */
    char battery_perc_str[9];
    uint8_t battery_perc = 95;
    snprintf(battery_perc_str, 9, "BAT:%d%%", battery_perc);
    draw_string(EINK_WIDTH - (8 * strlen(battery_perc_str)), 0, battery_perc_str, BLACK);

    /* Current title */
    char title[20] = "SELECT A DECK";
    draw_string(0, 0, title, BLACK);

    draw_hline(0, 8, EINK_WIDTH, BLACK);

    /* Draw menu */
    /* TODO: I need to only render however many decks on the screen that can fit at one
     * time based on the height, and then scroll if we hit the bottom*/
    uint8_t num_decks = 6;
    char deck_names[6][20] = {"BIOL 10100", "PHIL 32200", "ECE 20002", "ANTH 33700", "ECE 47700", "CS 15900"};

    for (uint8_t i = 0; i < num_decks; ++i) {
        if (i == curr_selected_deck) {
            /* Draw filled & inverted rectangle */
            draw_centered_string_in_filled_rect(0 + 50, 20 + DIST_DECKS * i, EINK_WIDTH - 1 - 50, 60 + DIST_DECKS * i, deck_names[i], BLACK);
        } else {
            /* Normal */
            draw_centered_string_in_rect(0 + 50, 20 + DIST_DECKS * i, EINK_WIDTH - 1 - 50, 60 + DIST_DECKS * i, deck_names[i], BLACK);
        }
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
