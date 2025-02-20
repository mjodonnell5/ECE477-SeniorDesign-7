#ifndef STATE_H
#define STATE_H

#include <stdint.h>

enum STATES {
    STATE_DOWNLOAD = 0, /* DISABLE INTERRUPTS */
    STATE_MENU_NAVIGATION,
    STATE_FLASHCARD_NAVIGATION,
    STATE_DECK_SELECTION
};

extern volatile enum STATES state;
extern volatile uint8_t render;

void state_machine();

#endif /* STATE_H */
