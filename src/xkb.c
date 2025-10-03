#include "xkb.h"
#include "state.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uchar.h>
#include <xkbcommon/xkbcommon.h>
#define DOUBLEKEYLIMIT 250 // in ms

void state_append_input (state_t *state, char32_t key);

void state_backspace_input (state_t *state, int pos);

void
xkb_handle_quick_double_key (state_t *state, clock_t current_time, char32_t key)
{
    xkb_t *xkb = state->xkb;
    if (current_time - xkb->time_of_last_key > DOUBLEKEYLIMIT)
        return state_append_input (state, key); // return the result of void (i dont want to have to add braces)

    bool has_binding = false;

    // TODO: real logic and not just this
    if (xkb->last_key == 'q' && key == 'q')
        {
            has_binding  = true;
            state->close = true;
        }

    if (xkb->last_key == 'e' && key == 'e')
        has_binding = true;

    if (has_binding)
        state_backspace_input (state, strlen (state->current_command_string) - 1);
    else
        state_append_input (state, key);
}

void
state_append_input (state_t *state, char32_t key)
{
    char *new_str = calloc (1, strlen (state->current_command_string) + sizeof (key) + sizeof ('\0'));
    sprintf (new_str, "%s%c", state->current_command_string, key);
    free (state->current_command_string);

    state->current_command_string = new_str;
}

void
state_backspace_input (state_t *state, int pos)
{
    if (!state->current_command_string || strcmp (state->current_command_string, "") == 0)
        return;

    char *new_str
        = malloc (strlen (state->current_command_string) * sizeof (char)); // ignoring \0 so that it gets smaller :3

    // avoids (NULL) in the string
    if (strlen (state->current_command_string) == 1)
        sprintf (new_str, "");

    // loop through and append each to new_str except for pos
    for (int i = 0; i < strlen (state->current_command_string); i++)
        if (i != pos)
            sprintf (&new_str[i], "%c", state->current_command_string[i]);

    free (state->current_command_string);
    state->current_command_string = new_str;
}

void
xkb_handle_key (state_t *state, uint32_t keycode)
{
    int64_t current_time = ms_since_epoch ();

    xkb_t *xkb       = state->xkb;
    xkb_keysym_t sym = xkb_state_key_get_one_sym (xkb->state, keycode);

    char buffer[128];
    xkb_keysym_get_name (sym, buffer, sizeof (buffer));
    xkb_state_key_get_utf8 (xkb->state, keycode, buffer, sizeof (buffer));

    char32_t utf = buffer[0]; // TODO: is this memory safe?
    IN_MESSAGE ("utf-8: %-4c (real? %i) (sym %d) (kc %u)", utf, !!utf, sym, keycode);

    if (!utf)
        return;
    if (keycode == 22)                                                             // backspace
        state_backspace_input (state, strlen (state->current_command_string) - 1); // TODO: selecting ?

    else if (xkb->time_of_last_key)
        xkb_handle_quick_double_key (state, current_time, utf);

    else
        state_append_input (state, utf);

    xkb->time_of_last_key = current_time;
    xkb->last_key         = utf;
    INFO ("current command: %s, %lu", state->current_command_string, strlen (state->current_command_string));
}

void
xkb_set_state (xkb_t *xkb)
{
    INFO ("xkb_set_state");
    if (xkb->state)
        xkb_state_unref (xkb->state);

    xkb->state = xkb_state_new (xkb->keymap);
}

void
xkb_set_keymap (xkb_t *xkb, char *keymap_str)
{
    INFO ("xkb_set_keymap");
    if (xkb->keymap)
        xkb_keymap_unref (xkb->keymap);

    xkb->keymap
        = xkb_keymap_new_from_string (xkb->context, keymap_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

    xkb_set_state (xkb);
}

xkb_t *
xkb_init ()
{
    xkb_t *xkb   = calloc (1, sizeof (xkb_t));
    xkb->context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    return xkb;
}
