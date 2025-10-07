#include "exec.h"
#include "config.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <unistd.h>

uint8_t
exec_cmd (char *cmd)
{
    char *argv[] = { "/bin/sh", "-c", cmd, NULL };

    execv (argv[0], argv);
    die (":3");
    return 0;
}

void
run (state_t *state)
{
    exec_cmd (&state->current_command_string[sizeof (INITIAL_COMMAND) - 1]); // -1 cause newline
}
