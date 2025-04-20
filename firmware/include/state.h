#ifndef STATE_H
#define STATE_H

#include <stdint.h>
#include <stddef.h>

#include "handlers.h"
#include "ui.h"

enum STATES {
    STATE_DOWNLOAD = 0, /* DISABLE INTERRUPTS */
    STATE_DECK_NAVIGATION,
    STATE_HOME_NAVIGATION,
    STATE_DELETE_DECK_CONFIRM,
    STATE_SETTINGS,
    STATE_DECK_HOME_PAGE,
    STATE_FLASHCARD_NAVIGATION,
    STATE_SLEEPING,
    NUM_STATES
};

enum EVENT {
    EVENT_NONE = 0,
    EVENT_BUTTON_UP_SHORT,
    EVENT_BUTTON_UP_LONG,
    EVENT_BUTTON_DOWN_SHORT,
    EVENT_BUTTON_DOWN_LONG,
    EVENT_BUTTON_SEL_SHORT,
    EVENT_BUTTON_SEL_LONG,
    NUM_EVENTS
};

typedef void (*event_handler)(void);

/* FIXME: Remove the NULL incase we homehow get there, don't want to dereference
 * a NULL ptr. Add an empty function or someting. */
static event_handler event_handlers[NUM_STATES][NUM_EVENTS] = {
    [STATE_HOME_NAVIGATION] = {
        [EVENT_BUTTON_UP_SHORT] = home_up_short,
        [EVENT_BUTTON_UP_LONG] = NULL,
        [EVENT_BUTTON_DOWN_SHORT] = home_down_short,
        [EVENT_BUTTON_DOWN_LONG] = NULL,
        [EVENT_BUTTON_SEL_SHORT] = home_sel_short,
        [EVENT_BUTTON_SEL_LONG] = NULL,
    },
    [STATE_DECK_NAVIGATION] = {
        [EVENT_BUTTON_UP_SHORT] = deck_up_short,
        [EVENT_BUTTON_UP_LONG] = NULL,
        [EVENT_BUTTON_DOWN_SHORT] = deck_down_short,
        [EVENT_BUTTON_DOWN_LONG] = deck_up_short,
        [EVENT_BUTTON_SEL_SHORT] = deck_sel_short,
        [EVENT_BUTTON_SEL_LONG] = deck_sel_long,
    },
    [STATE_DELETE_DECK_CONFIRM] = {
        [EVENT_BUTTON_UP_SHORT] = NULL,
        [EVENT_BUTTON_UP_LONG] = NULL,
        [EVENT_BUTTON_DOWN_SHORT] = NULL,
        [EVENT_BUTTON_DOWN_LONG] = delete_down_long,
        [EVENT_BUTTON_SEL_SHORT] = NULL,
        [EVENT_BUTTON_SEL_LONG] = delete_sel_long,
    },
    [STATE_SETTINGS] = {
        [EVENT_BUTTON_UP_SHORT] = NULL,
        [EVENT_BUTTON_UP_LONG] = settings_up_short,
        [EVENT_BUTTON_DOWN_SHORT] = settings_down_short,
        [EVENT_BUTTON_DOWN_LONG] = settings_down_long,
        [EVENT_BUTTON_SEL_SHORT] = settings_sel_short,
        [EVENT_BUTTON_SEL_LONG] = NULL,
    },
    [STATE_FLASHCARD_NAVIGATION] = {
        [EVENT_BUTTON_UP_SHORT] = flashcard_up_short,
        [EVENT_BUTTON_UP_LONG] = NULL,
        [EVENT_BUTTON_DOWN_SHORT] = flashcard_down_short,
        [EVENT_BUTTON_DOWN_LONG] = flashcard_down_long,
        [EVENT_BUTTON_SEL_SHORT] = flashcard_sel_short,
        [EVENT_BUTTON_SEL_LONG] = NULL,
    },
};

#define EVQ_SIZE 4
struct evq {
  volatile uint8_t head, tail;
  enum EVENT buf[EVQ_SIZE];
};

enum EVENT evq_pop();
uint8_t evq_push(enum EVENT e);

/* FIXME: I need to get rid of all of this global state, its left over from
 * previous state machine stuff and instead of cleaning it up I am just passing
 * it along to the handlers for now */

extern volatile enum STATES state;
extern volatile uint8_t render_pending;
extern struct font curr_font;
extern uint8_t left_handed;
extern volatile struct evq q;
extern volatile uint8_t curr_menu_selection;
extern uint8_t num_decks;
extern char deck_names[MAX_DECKS][MAX_NAME_SIZE];
extern uint8_t fetch_decks;
extern volatile uint8_t shuffle;
extern volatile uint8_t font_large;
extern struct deck main_deck;

void state_machine();

#endif /* STATE_H */
