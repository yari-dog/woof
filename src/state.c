#include "state.h"
#include "exec.h"
#include "render.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

state_t *
init_state ()
{
    state_t *state                = calloc (1, sizeof (state_t));

    state->ccs_buffsize           = BUFFSIZE + NULL_TERM_SIZE; // add null ter
    state->current_command_string = calloc (1, BUFFSIZE + NULL_TERM_SIZE);

    strcpy (state->current_command_string, INITIAL_COMMAND);
    state->cursor            = strlen (state->current_command_string);
    state->render_context    = render_init (state);
    state->update            = false;
    state->run_time          = ms_since_epoch ();
    state->cur_time          = ms_since_epoch ();
    state->cur_visible       = true;
    state->typing            = false;
    state->time_since_typing = ms_since_epoch ();

    get_results (state);

    return state;
}
