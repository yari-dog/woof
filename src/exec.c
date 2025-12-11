#include "exec.h"
#include "state.h"
#include "util.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

    // attain list of locations to look for .desktop files
    char *xdg_data_dirs;
    if ((xdg_data_dirs = getenv ("XDG_DATA_DIRS")) == NULL)
        die ("null xdg_data_dirs");

    char *dirdelim = strstr (xdg_data_dirs, ":");

    char dirstr[256];
    DIR *d;
    struct dirent *dir;

    // search those dirs for .desktop files

    while (xdg_data_dirs != NULL)
        {
            if (dirdelim != NULL)
                *dirdelim++ = '\0';

            sprintf (dirstr, "%s/applications", xdg_data_dirs);
            INFO ("data dir in: %s", dirstr);

            // if <x>/applications dir exists
            if (!access (dirstr, R_OK) && (d = opendir (dirstr)) != NULL)
                {
                    // iterate through the files in there
                    while ((dir = readdir (d)) != NULL)
                        {
                            // if its a .desktop file
                            if (strstr (dir->d_name, ".desktop") == NULL)
                                continue;

                            INFO ("file: %s", dir->d_name);
                        }
                    closedir (d);
                }

            // swap to next dir, check if its the last-1 or last dir.
            xdg_data_dirs = dirdelim;
            if (dirdelim != NULL && (dirdelim = strstr (xdg_data_dirs, ":")) == NULL)
                dirdelim = NULL;
        }

    sort_results (state);
}

void
run (state_t *state)
{
    exec_cmd (&state->current_command_string[5]); // remove ":run "
}
