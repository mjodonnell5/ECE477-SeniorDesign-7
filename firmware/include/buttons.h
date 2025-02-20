#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

void button_init();

extern volatile uint8_t curr_deck_selection;
extern volatile uint8_t curr_card_selection;
extern volatile uint8_t get_deck_from_sd;
extern volatile uint8_t f_b;

#endif /* BUTTONS_H */
