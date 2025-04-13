#ifndef STATE_H
#define STATE_H

#include <stdint.h>

enum STATES {
    STATE_DOWNLOAD = 0, /* DISABLE INTERRUPTS */
    STATE_MENU_NAVIGATION,
    STATE_HOME_NAVIGATION,
    STATE_DELETE_DECK_CONFIRM,
    STATE_SETTINGS,
    STATE_DECK_HOME_PAGE,
    STATE_FLASHCARD_NAVIGATION,
    STATE_SLEEPING,
};

extern volatile enum STATES state;
extern volatile uint8_t render_pending;

void state_machine();

#endif /* STATE_H */
