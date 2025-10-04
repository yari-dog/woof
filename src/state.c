#include "state.h"
#include "render.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#define INITIAL_COMMAND ":"

state_t *
init_state ()
{
    state_t *state = calloc (1, sizeof (state_t));

    state->ccs_buffsize           = BUFFSIZE + NULL_TERM_SIZE; // add null ter
    state->current_command_string = calloc (1, BUFFSIZE + NULL_TERM_SIZE);

    strcpy (state->current_command_string, INITIAL_COMMAND);
    state->cursor         = strlen (state->current_command_string);
    state->render_context = render_init (state);

    return state;
}
