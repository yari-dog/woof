#include "exec.h"
#include "state.h"
#include "util.h"
#include "woof.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t
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

static result_t *
prev_result (result_t *result)
{
    return result->prev;
}

static result_t *
next_result (result_t *result)
{
    return result->next;
}

void
move_hover (state_t *state, int8_t change)
{
    result_t *result = state->hovered_result;
    result->hovered  = false;
    result_t *(*step) (result_t *);

    if (change < 0)
        step = prev_result;
    else
        step = next_result;

    while (change)
        {
            if (!step (result))
                break;

            result  = step (result);

            change -= (((unsigned)change >> 7) | (!!change)); // this adds 1 for a negative change and vice versa
        }

    state->hovered_result = result;
    result->hovered       = true;
}

static void
sort_results (state_t *state)
// make a linked list of all matching results from array of all ?
{
    result_t **results = state->results;
    result_t *result   = (*results);
    result_t *l_result = NULL;

    // init just do this for time being
    result->hovered       = true;
    state->hovered_result = result;
    state->head_result    = result;

    // replace with actual sorting when i figure ts out
    for (int i = 0; i < state->result_count; i++, results++)
        {
            result          = (*results);
            result->visible = true;
            result->pos     = i;
            if (l_result)
                {
                    result->prev   = l_result;
                    l_result->next = result;
                }
            l_result = result;
        }
}

// how the fuck do i do this lmao
// i think if i check the dirs in $XDG_DATA_DIRS for applications/*.desktop that should work
void
get_results (state_t *state)
{
    // attain list of locations to look for .desktop files
    char *xdg_data_dirs;
    if ((xdg_data_dirs = getenv ("XDG_DATA_DIRS")) == NULL)
        die ("null xdg_data_dirs");

    char *dirdelim = strstr (xdg_data_dirs, ":");

    char dirstr[256];
    DIR *d;
    struct dirent *dir;

    state->results     = calloc (256, sizeof (result_t *));
    result_t **results = state->results;

    // search those dirs for .desktop files
    while (xdg_data_dirs != NULL)
        {
            if (dirdelim != NULL)
                *dirdelim++ = '\0';

            sprintf (dirstr, "%s/applications", xdg_data_dirs);

            // if <x>/applications dir exists
            if (!access (dirstr, R_OK) && (d = opendir (dirstr)) != NULL)
                {
                    // iterate through the files in there
                    while ((dir = readdir (d)) != NULL)
                        {
                            // if its a .desktop file
                            if (strstr (dir->d_name, ".desktop") == NULL)
                                continue;

                            state->result_count++;

                            // create result and append pointer to state->results
                            *results               = calloc (1, sizeof (result_t));

                            (*results)->entry_path = calloc (1, strlen (dirstr) + strlen (dir->d_name) + 2);
                            sprintf ((*results)->entry_path, "%s/%s", dirstr, dir->d_name);

                            FILE *f;
                            char *key;
                            size_t length = 0;
                            ssize_t read;

                            // read the desktop files
                            if ((f = fopen ((*results)->entry_path, "r")) == NULL)
                                die ("%s not opening :(", (*results)->entry_path);

                            while ((read = getline (&key, &length, f)) != -1)
                                {
                                    char *val_ptr = strstr (key, "=");

                                    if (val_ptr == NULL)
                                        continue;

                                    *val_ptr++                = '\0';

                                    *(strstr (val_ptr, "\n")) = '\0'; // getline keeps newline
                                                                      // there might be a scenario where the line might
                                                                      // not have a new line but idgaf

                                    char *val  = calloc (1, strlen (val_ptr) + 1);
                                    char **ptr = NULL;

                                    sprintf (val, "%s", val_ptr);
                                    if (strcmp (key, "Name") == 0)
                                        ptr = &(*results)->title;
                                    else if (strcmp (key, "Exec") == 0)
                                        ptr = &(*results)->exec_cmd;
                                    else if (strcmp (key, "Comment") == 0)
                                        ptr = &(*results)->comment;
                                    else if (strcmp (key, "Path") == 0)
                                        ptr = &(*results)->path;
                                    else if (strcmp (key, "Terminal") == 0)
                                        (*results)->terminal = strcmp (val_ptr, "true") ? true : false;

                                    if (ptr == NULL)
                                        free (val);
                                    else
                                        *ptr = val;
                                }

                            fclose (f);

                            results++;
                        }
                    closedir (d);
                }

            // swap to next dir, check if its the last-1 or last dir.
            xdg_data_dirs = dirdelim;
            if (dirdelim != NULL && (dirdelim = strstr (xdg_data_dirs, ":")) == NULL)
                dirdelim = NULL;
        }

    // result_t **t_results = state->results;
    // for (int i = 0; i < state->result_count; i++, t_results++)
    //     INFO ("found: %s", (*t_results)->title);

    sort_results (state);
    move_hover (state, 0); // select first in list (they are all visible at start, so first is chilling)
}

// todo: running yazi does not work
static void
exec_desktop (state_t *state)
{
    result_t *result = state->hovered_result;
    char *exec       = result->exec_cmd;
    exec_cmd (exec);
}

void
run (state_t *state)
{
    if (strstr (state->current_command_string, ":run ") == state->current_command_string)
        exec_cmd (&state->current_command_string[5]); // remove ":run "
    else
        exec_desktop (state);
}
