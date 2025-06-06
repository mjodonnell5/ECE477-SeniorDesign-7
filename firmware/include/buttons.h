#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

#define LONG_PRESS_TIME_MS (500)

#define NO_PRESS    (0)
#define SHORT_PRESS (1)
#define LONG_PRESS  (2)

#define SELECT   (0)
#define BACKWARD (1)
#define FORWARD  (2)

void button_init();
void disable_buttons();
void enable_buttons();

extern volatile uint8_t curr_deck_selection;
extern volatile uint8_t curr_card_selection;
extern volatile uint8_t get_deck_from_sd;
extern volatile uint8_t f_b;

#endif /* BUTTONS_H */
