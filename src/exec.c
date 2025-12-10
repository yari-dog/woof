#include "exec.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <unistd.h>

uint8_t
exec_cmd (char *cmd)
{
    char *argv[] = { "/bin/sh", "-c", cmd, NULL };

    switch (fork ())
        {
        case -1:
            die ("fork:");
        case 0:
            execv (argv[0], argv);
            die ("bye bye :3");
        }
    return 0;
}

void
sort_results (state_t *state)
{
}

// how the fuck do i do this lmao
// i think if i check the dirs in $XDG_DATA_DIRS for applications/*.desktop that should work
void
get_results (state_t *state)
{
    state->results = calloc (5, sizeof (result_t));

    sort_results (state);
}

void
run (state_t *state)
{
    exec_cmd (&state->current_command_string[5]); // remove ":run "
}
