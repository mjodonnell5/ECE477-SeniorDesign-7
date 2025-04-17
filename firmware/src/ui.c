#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32l496xx.h>

#include "../include/eink.h"
#include "../include/clock.h"
#include "../include/ui.h"
#include "../include/font.h"
#include "../include/battery.h"
#include "../include/rtc.h"
#include "cJSON.h"

#define MENU_ITEM_DIST (50)
#define MENU_ITEM_HEIGHT (40)

extern uint8_t font8x8_basic[128][8];
extern struct deck main_deck;

extern uint8_t zzz[];
#define IMAGE_WIDTH 200
#define IMAGE_HEIGHT 200

void draw_sleep_image(uint16_t s_x, uint16_t s_y)
{
    for (uint16_t y = 0; y < IMAGE_HEIGHT; y++) {
        for (uint16_t x = 0; x < IMAGE_WIDTH; x++) {
            uint32_t byte_index = (y * IMAGE_WIDTH + x) / 8;
            uint8_t bit_position = (x % 8);
            uint8_t pixel_value = (zzz[byte_index] >> (7 - bit_position)) & 1;
            if (pixel_value) {
                eink_draw_pixel(s_x + x, s_y + y, BLACK);
            }
        }
    }
}
void draw_header(char* title)
{
    /* Current title */
    draw_centered_string_in_filled_rect(large_font, 0, 0, EINK_WIDTH - 1, 15, title, BLACK);

    /* Battery percentage */
    // uint16_t soc = BQ27441_ReadWord(BQ27441_COMMAND_SOC);
    char battery_perc_str[10];
    char chg_str[6] = "CHRG ";
    uint8_t battery_perc = 100;
    snprintf(battery_perc_str, sizeof(battery_perc_str), "%s%d%%", is_charging() ? chg_str : "", battery_perc);
    draw_string(large_font, EINK_WIDTH - (8 * strlen(battery_perc_str)), 0, battery_perc_str, WHITE);
}

void draw_menu(uint8_t curr_selected, char names[][MAX_NAME_SIZE], uint16_t num)
{
    uint8_t curr_page = curr_selected / MAX_ITEMS_PER_PAGE;
    uint8_t decks_on_page = num - (curr_page * MAX_ITEMS_PER_PAGE);

    /* If curr_page == 0 */
    if (decks_on_page > MAX_ITEMS_PER_PAGE) {
        decks_on_page = MAX_ITEMS_PER_PAGE;
    }

    /* There is only distance after an item if there is another item after it */
    // uint16_t total_menu_height = (MENU_ITEM_HEIGHT * num) + (MENU_ITEM_DIST * (num - 1));
    // uint16_t y_start_offset = (EINK_HEIGHT - total_menu_height) / 2;
    uint16_t y_start_offset = 0;

    // char card_str[100];
    // snprintf(card_str, 100, "Cards: %d");

    for (uint8_t i = 0; i < decks_on_page; ++i) {
        uint8_t index = i + (curr_page * MAX_ITEMS_PER_PAGE);
        if (index == curr_selected) {
            /* Draw filled & inverted rectangle */
            draw_centered_string_in_filled_rect(large_font, 50, 20 + MENU_ITEM_DIST * i, EINK_WIDTH - 50, (20 + MENU_ITEM_HEIGHT) + MENU_ITEM_DIST * i, names[index],  BLACK);
            // draw_filled_deck_menu_item(large_font, 0 + 50, 20 + MENU_ITEM_DIST * i, EINK_WIDTH - 1 - 50, 60 + MENU_ITEM_DIST * i, names[index], "Cards: 100 | Last Studied: 03/19", BLACK);
        } else {
            /* Normal */
            draw_centered_string_in_rect(large_font, 50, 20 + MENU_ITEM_DIST * i, EINK_WIDTH - 50, (20 + MENU_ITEM_HEIGHT) + MENU_ITEM_DIST * i, names[index], BLACK);
            // draw_deck_menu_item(large_font, 0 + 50, 20 + MENU_ITEM_DIST * i, EINK_WIDTH - 1 - 50, 60 + MENU_ITEM_DIST * i, names[index], "Cards: 100 | Last Studied: 03/19", BLACK);
        }
    }
}

void draw_main_menu(uint8_t curr_selected_deck, char deck_names[][MAX_NAME_SIZE], uint16_t num_decks)
{
    uint8_t curr_page = curr_selected_deck / MAX_ITEMS_PER_PAGE;
    uint8_t decks_on_page = num_decks - (curr_page * MAX_ITEMS_PER_PAGE);

    /* If curr_page == 0 */
    if (decks_on_page > MAX_ITEMS_PER_PAGE) {
        decks_on_page = MAX_ITEMS_PER_PAGE;
    }

    /* Need to make this happen and update once a deck is accessed */
    char studied_date[6];
    read_rtc(&dt);
    snprintf(studied_date, sizeof(studied_date), "%02d/%02d", dt.mon, dt.day);

    /* FIXME: Need to make this per deck */
    char metadata[50];
    uint16_t num_cards = 25;
    snprintf(metadata, sizeof(metadata), "Cards: %d | Studied: %s", num_cards, studied_date);

    //accessing metadata info
    FILE* dataFile = fopen("metadata.txt", "r");
    if (!dataFile) {
        return;
    }

    char jsonBuffer[4096];
    size_t bytesRead = fread(jsonBuffer, 1, sizeof(jsonBuffer) - 1, dataFile);
    fclose(dataFile); //close datafile
    jsonBuffer[bytesRead] = '\0'; //null terminate

    cJSON* metadataJSON = cJSON_Parse(jsonBuffer);
    if (!metadataJSON) {
        return;
    }

    cJSON* setsArray = cJSON_GetObjectItem(metadataJSON, "sets");
    if (!cJSON_IsArray(setsArray)) {
        printf("Error: Invalid metadata structure\n");
        cJSON_Delete(metadataJSON);
        return;
    }

    //iterate though the decks on the page
    for (uint8_t i = 0; i < decks_on_page; ++i) {
        uint8_t index = i + (curr_page * MAX_ITEMS_PER_PAGE);

        //finding metadata for current deck
        cJSON* set = NULL;
        cJSON_ArrayForEach(set, setsArray) {
            cJSON* setName = cJSON_GetObjectItem(set, "setName");
            if (cJSON_IsString(setName) && strcmp(setName->valuestring, deck_names[index]) == 0) {
                //extract numCards and last_studied
                cJSON* numCards = cJSON_GetObjectItem(set, "numCards");
                uint16_t numCardsInt = cJSON_IsNumber(numCards) ? numCards->valueint : 0; //converting to int
                // cJSON* lastStudied = cJSON_GetObjectItem(set, "last_studied");
                // uint32_t lastStudiedInt = lastStudied->valueint;
                // snprintf(lastStudiedStr, sizeof(studied_date), "%02d/%02d", dt.mon, dt.day);
                char studied_date[6];
                read_rtc(&dt);
                snprintf(studied_date, sizeof(studied_date), "%02d/%02d", dt.mon, dt.day);

                char metadata[50];
                snprintf(metadata, sizeof(metadata), "Cards: %d | Studied: %s", numCardsInt, studied_date);
            }
        

            if (index == curr_selected_deck) {
                /* Draw filled & inverted rectangle */
                draw_filled_deck_menu_item(large_font, 0 + 50, 20 + MENU_ITEM_DIST * i, EINK_WIDTH - 1 - 50, 60 + MENU_ITEM_DIST * i, deck_names[index], metadata, BLACK);
            } else {
                /* Normal */
                draw_deck_menu_item(large_font, 0 + 50, 20 + MENU_ITEM_DIST * i, EINK_WIDTH - 1 - 50, 60 + MENU_ITEM_DIST * i, deck_names[index], metadata, BLACK);
            }
        }
    }
    cJSON_Delete(metadataJSON); //delete JSON object
}

void draw_flashcard(struct flashcard fc, uint8_t f_b, uint8_t col)
{
    if (f_b) {
        draw_centered_string_wrapped(large_font, fc.front, col);
    } else {
        draw_centered_string_wrapped(large_font, fc.back, col);
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

void draw_char(struct font f, uint16_t s_x, uint16_t s_y, uint16_t c, uint8_t col)
{ 
    uint8_t bytes_per_row = (f.width + 7) / 8;
    const uint8_t* bitmap = &f.font[c * f.height * bytes_per_row];

    for (uint8_t y = 0; y < f.height * bytes_per_row; ++y) {
        for (uint8_t x = 0; x < f.width; ++x) {
            uint8_t is_set = bitmap[y] & (1 << (7 - x));
            if (is_set) {
                eink_draw_pixel(s_x + x, s_y + y, col);
            }
        }
    }
}

void draw_string(struct font f, uint16_t s_x, uint16_t s_y, char* string, uint8_t col)
{
    while (*string) {
        draw_char(f, s_x, s_y, *string, col);

        string++;
        s_x += 8;
    }
}

void draw_string_wrapped(struct font f, uint16_t s_x, uint16_t s_y, char* string, uint8_t col) {
    uint16_t c_x = s_x;
    uint16_t c_y = s_y;

    char buffer[2048];
    strncpy(buffer, string, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* word = strtok(buffer, " ");

    while (word != NULL) {
        uint16_t word_length_pixels = strlen(word) * f.width;

        /* subtracting c_x will "center" it so that it wraps not at the end
         * of the display, but so that the text is centered. */
        // if (word_length_pixels + c_x >= EINK_WIDTH - s_x) { (old centering mode)

        if (word_length_pixels + c_x >= EINK_WIDTH) {
            /* Write this word on next line */
            c_x = s_x;
            // c_y += 10;
            c_y += f.height;
        }

        draw_string(f, c_x, c_y, word, col);

        /* +8 is to add a space after the word*/
        c_x += word_length_pixels + 8;

        word = strtok(NULL, " ");
    }
}

void draw_centered_string_in_filled_rect(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col)
{
    uint16_t min_x = e_x > s_x ? s_x : e_x;
    uint16_t max_x = e_x > s_x ? e_x : s_x;
    uint16_t min_y = e_y > s_y ? s_y : e_y;
    uint16_t max_y = e_y > s_y ? e_y : s_y;

    uint16_t width = max_x - min_x + 1;
    uint16_t height = max_y - min_y + 1;
    
    uint16_t text_width = strlen(string) * f.width;
    uint8_t text_height = f.height;

    uint16_t text_x = min_x + (width - text_width) / 2;
    uint16_t text_y = min_y + (height - text_height) / 2;

    draw_filled_rect(min_x, min_y, max_x, max_y, col);

    draw_string(f, text_x, text_y, string, !col);
}

void draw_filled_deck_menu_item(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, char* string2, uint8_t col)
{
    uint16_t min_x = e_x > s_x ? s_x : e_x;
    uint16_t max_x = e_x > s_x ? e_x : s_x;
    uint16_t min_y = e_y > s_y ? s_y : e_y;
    uint16_t max_y = e_y > s_y ? e_y : s_y;

    uint16_t width = max_x - min_x + 1;
    uint16_t height = max_y - min_y + 1;
    
    uint16_t text_width = strlen(string) * f.width;
    uint8_t text_height = f.height;

    uint16_t text_x = min_x + (width - text_width) / 2;
    uint16_t text_y = min_y + (height - f.height - text_height) / 2;

    draw_filled_rect(min_x, min_y, max_x, max_y, col);

    draw_string(f, 65, text_y + f.height / 2, "> ", !col);
    draw_string(f, 85, text_y, string, !col);
    draw_string(small_font, 85, text_y + f.height + 4, string2, !col);
}

void draw_centered_string_in_rect(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, uint8_t col)
{
    uint16_t min_x = e_x > s_x ? s_x : e_x;
    uint16_t max_x = e_x > s_x ? e_x : s_x;
    uint16_t min_y = e_y > s_y ? s_y : e_y;
    uint16_t max_y = e_y > s_y ? e_y : s_y;

    uint16_t width = max_x - min_x + 1;
    uint16_t height = max_y - min_y + 1;
    
    uint16_t text_width = strlen(string) * f.width;
    uint8_t text_height = f.height;

    uint16_t text_x = min_x + (width - text_width) / 2;
    uint16_t text_y = min_y + (height - text_height) / 2;

    draw_rect(min_x, min_y, max_x, max_y, col);

    draw_string(f, text_x, text_y, string, col);
}

void draw_deck_menu_item(struct font f, uint16_t s_x, uint16_t s_y, uint16_t e_x, uint16_t e_y, char* string, char* string2, uint8_t col)
{
    uint16_t min_x = e_x > s_x ? s_x : e_x;
    uint16_t max_x = e_x > s_x ? e_x : s_x;
    uint16_t min_y = e_y > s_y ? s_y : e_y;
    uint16_t max_y = e_y > s_y ? e_y : s_y;

    uint16_t width = max_x - min_x + 1;
    uint16_t height = max_y - min_y + 1;
    
    uint16_t text_width = strlen(string) * f.width;
    uint8_t text_height = f.height;

    uint16_t text_x = min_x + (width - text_width) / 2;
    uint16_t text_y = min_y + (height - f.height - text_height) / 2;

    draw_rect(min_x, min_y, max_x, max_y, col);

    draw_string(f, 85, text_y, string, col);
    draw_string(small_font, 85, text_y + f.height + 4, string2, col);
}

void draw_centered_string_wrapped(struct font f, char* string, uint8_t col)
{
    uint16_t text_width = strlen(string) * f.width;
    uint8_t num_wraps = text_width / (EINK_WIDTH - 20);
    uint8_t height = (EINK_HEIGHT - (num_wraps * f.height)) / 2;

    uint8_t width = (EINK_WIDTH - text_width) / 2;
    if (num_wraps > 0) width = 20;

    draw_string_wrapped(f, width, height, string, col);
}
