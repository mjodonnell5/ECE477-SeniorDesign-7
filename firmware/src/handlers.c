#include "../include/state.h"
#include "../include/buttons.h"
#include "../include/commands.h"

void none()
{
    return;
}

/* HOME NAVIGATION */
void home_sel_short()
{
    if (curr_menu_selection == 0) {
        state = STATE_DECK_NAVIGATION;
    } else if (curr_menu_selection == 1) {
        state = STATE_DOWNLOAD;
    } else if (curr_menu_selection == 2) {
        state = STATE_SETTINGS;
    }
}

void home_down_short()
{
    if (curr_menu_selection > 0) {
        curr_menu_selection--;
    }
}

void home_up_short()
{
    if (curr_menu_selection + 1 < 3) {
        curr_menu_selection++;
    }
}


/* DECK NAVIGATION */
void deck_sel_short()
{
    state = STATE_FLASHCARD_NAVIGATION;
    get_deck_from_sd = 1;
}

void deck_sel_long()
{
    state = STATE_DELETE_DECK_CONFIRM;
}

void deck_down_short()
{
    if (curr_deck_selection > 0) {
        curr_deck_selection--;
    }
}

void deck_down_long()
{
    state = STATE_HOME_NAVIGATION;
}

void deck_up_short()
{
    if (curr_deck_selection + 1 < num_decks) {
        curr_deck_selection++;
    }
}

/* DELETE DECK CONFIRM */
void delete_sel_long()
{
    /* Delete highlighted deck */
    delete_file(deck_names[curr_deck_selection]);
    fetch_decks = 1;
    state = STATE_DECK_NAVIGATION;
}

void delete_down_long()
{
    state = STATE_DECK_NAVIGATION;
}

/* SETTINGS */
void settings_sel_short()
{
    shuffle = !shuffle;
}

void settings_down_short()
{
    left_handed = !left_handed;
}

void settings_down_long()
{
    state = STATE_HOME_NAVIGATION;
}

void settings_up_short()
{
    font_large = !font_large;
}

/* FLASHCARD */
void flashcard_sel_short()
{
    /* Flip */
    f_b = !f_b;
}

void flashcard_down_short()
{
    if (curr_card_selection > 0) {
        curr_card_selection--;
    }

    /* Always start on the front */
    if (f_b == BACK) f_b = FRONT;
}

void flashcard_down_long()
{
    state = STATE_DECK_NAVIGATION;

    /* Don't save previous state when going back in the menu */
    curr_deck_selection = 0;
    curr_card_selection = 0;
    f_b = FRONT;
}

void flashcard_up_short()
{
    if (curr_card_selection + 1 < main_deck.num_cards) {
        curr_card_selection++;
    }

    /* Always start on the front */
    if (f_b == BACK) f_b = FRONT;
}
