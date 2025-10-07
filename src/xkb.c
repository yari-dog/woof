#include "xkb.h"
#include "config.h"
#include "exec.h"
#include "state.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uchar.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

size_t
next_rune (state_t *state, int n)
{
    // i stole ts entirely from mew
    ssize_t i;
    for (i = state->cursor + n; i + n >= 0 && (state->current_command_string[i] & 0xc0) == 0x80; i += n)
        ;

    return i;
}

void
move_cursor (state_t *state, size_t n)
{
    if (state->cursor + n <= 0)
        n = state->cursor - 1;
    if (state->cursor + n > strlen (state->current_command_string))
        n = strlen (state->current_command_string) - state->cursor;

    state->cursor += n;
}

void
expand_command_str_buf (state_t *state, ssize_t n)
{

    INFO ("------------------------------- expanding buf");
    char *next = state->current_command_string, *prev = NULL;
    int next_size
        = state->ccs_buffsize + BUFFSIZE + n; // cba to make it exact. just have that buffsize added for padding.

    if ((next = (char *)realloc (prev = next, next_size)) == NULL)
        die ("realloc on ccs died :/");

    if (strcmp (state->current_command_string, prev) != 0)
        INFO ("realloc on css weird, %s %lu %s %lu", prev, strlen (prev), next, strlen (next));

    state->current_command_string  = next;
    state->ccs_buffsize           += BUFFSIZE; // just add the default on lmao
    INFO ("expanded buf --------------------------------");
}

// TODO: is the memory management here atrocious
void
insert (state_t *state, char *text, ssize_t n)
{
    // check if backspace will put cursor into the negative
    // (could probably do it branchlessly easily but that will be unreadable)
    if ((int)state->cursor + n < 0)
        n = MAX (0, (int)state->cursor + n);

    if (strlen (state->current_command_string) + NULL_TERM_SIZE + n > state->ccs_buffsize)
        expand_command_str_buf (state, n);

    //  imagine string is 1-10 , cursor is 6, and n is 5, as if we are inserting 5 characters in.
    //
    //
    //  first you gotta first move the stuff after out of the way.
    memmove (&state->current_command_string[state->cursor + n], &state->current_command_string[state->cursor],
             strlen (state->current_command_string) - state->cursor - MAX (0, n) + NULL_TERM_SIZE);
    //   1 2 3 4 5 6 (cursor) - - - - - (cursor + n) 7 8 9 10
    //   (the - represents junk)
    //
    //   next is inserting the new text
    if (text)
        memcpy (&state->current_command_string[state->cursor], text, n);

    // update cursor
    move_cursor (state, n);
}

void
xkb_handle_quick_double_key (state_t *state, clock_t current_time, char *buf)
{
    xkb_t *xkb = state->xkb;
    if (current_time - xkb->time_of_last_key > DOUBLEKEYLIMIT)
        return insert (state, buf, strnlen (buf, 8)); // return the result of void (i dont want to have to add braces)

    bool has_binding = false;

    // TODO: real logic and not just this
    // ig can do it either way
    if (xkb->last_key == 'q' && strcmp (buf, "q") == 0)
        {
            has_binding  = true;
            state->close = true;
        }

    // been using this to test
    if (xkb->last_key == 'e' && buf[0] == 'e')
        has_binding = true;

    if (has_binding)
        insert (state, NULL, next_rune (state, -1) - state->cursor);
    else
        insert (state, buf, strnlen (buf, 8)); // return the result of void (i dont want to have to add braces)
}

void
xkb_handle_key (state_t *state, uint32_t keycode)
{
    int64_t current_time = ms_since_epoch ();

    xkb_t *xkb       = state->xkb;
    xkb_keysym_t sym = xkb_state_key_get_one_sym (xkb->state, keycode);

    char buf[8];
    switch (sym)
        {
        case XKB_KEY_BackSpace:
            insert (state, NULL, next_rune (state, -1) - state->cursor); // TODO: selecting ?
            break;
        case XKB_KEY_Escape:
            state->close = true;
            break;
        case XKB_KEY_Return:
            state->close = true;
            run (state);
            break;
        case XKB_KEY_Left:
            move_cursor (state, -1);
            break;
        case XKB_KEY_Right:
            move_cursor (state, 1);
            break;
        default:
            if (xkb_keysym_to_utf8 (sym, buf, 8))
                xkb_handle_quick_double_key (state, current_time, buf);
        }
    xkb->time_of_last_key = current_time;
    xkb->last_key         = buf[0];

    state->update = true;

    IN_MESSAGE ("(sym %d) (kc %u)", sym, keycode);
    INFO ("current command: %s, %lu, %zu", state->current_command_string, strlen (state->current_command_string),
          state->cursor);
}

// V----------------------- im a little unsure if theese two ever will exist seperately. i dont think so tbh.
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
// ^--------------------------

xkb_t *
xkb_init ()
{
    xkb_t *xkb   = calloc (1, sizeof (xkb_t));
    xkb->context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    return xkb;
}
