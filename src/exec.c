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
run (state_t *state)
{
    exec_cmd (&state->current_command_string[5]); // remove ":run "
}
