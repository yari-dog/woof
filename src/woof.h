#ifndef WOOF_H
#define WOOF_H
#include "state.h"
#include "util.h"
#include "wayland/wayland.h"
#include "xkb.h"
#include <string.h>

#define WAYLAND "wayland"
#define X11     "CHANGE ME"
typedef struct woof_t
{
    void (*start) (state_t *);
    void (*main_loop) (state_t *);
    void (*cleanup) (state_t *);
    struct state_t *state;
} woof_t;

extern woof_t *g_woof;

static woof_t *
init_woof ()
{
    /*
     * woof gets initialised with a state, this holds some information about it's windowing stuff.
     * lotta this stuff has to be proxied thought a pointer to it's location in it's windowing manager's handle
     * such as the function for the main loop logic. this means that main() can be x/wayland agnostic
     */
    INFO ("woof initiating :o");
    g_woof                  = calloc (1, sizeof (woof_t));

    state_t *state          = init_state ();
    g_woof->state           = state;

    state->xkb              = xkb_init ();

    const char *xdg_backend = getenv ("XDG_BACKEND");
    // get display server
    // if (!(xdg_backend = getenv ("XDG_BACKEND")))
    //     die ("cant find backend lmao");

    // if (strcmp (xdg_backend, WAYLAND) == 0)
    if (getenv ("WAYLAND_DISPLAY") || strcmp (getenv ("XDG_SESSION_TYPE"), "wayland"))
        {
            state->wlc        = wlc_init ();
            g_woof->start     = wlc_start;
            g_woof->main_loop = wlc_main_loop;
            g_woof->cleanup   = wlc_disconnect;
        }
    // TODO: (x): implement this
    // xc_init() is the function that inits the struct, it doesnt implement any x setup
    // start is your actual x setup code (making the shm buffers, etc)
    // the others are comprehendable
    else if (strcmp (xdg_backend, X11) == 0)
        {
            // i dont know what that X11 macro should actually be lmao
            /* setup for x11 would be as follows:
             * state->xc            = xc_init ();
             * g_woof->start                = xc_start;
             * g_woof->main_loop            = xc_main_loop;
             * g_woof->cleanup              = xc_disconnect;
             */
            die ("uh wait"); // remove when x11
        }
    else
        die ("no display server ? :(");

    INFO ("woof initiated :3");
    return g_woof;
}

static void
destroy_woof (woof_t *woof)
{
    woof->cleanup (woof->state);

    // need to do xkb cleanup
    free (woof->state->xkb);
    free (woof->state);
    free (woof);
}
#endif
