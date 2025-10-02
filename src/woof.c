#include "woof.h"
#include "state.h"
#include "util.h"
#include "wayland/wayland.h"
#define WAYLAND 1
#define X11     0

woof_t *
init_woof ()
{
    /*
     * woof gets initialised with a state, this holds some information about it's windowing stuff.
     * lotta this stuff has to be proxied thought a pointer to it's location in it's windowing manager's handle
     * such as the function for the main loop logic. this means that main() can be x/wayland agnostic
     */
    INFO ("woof initiating :o");
    woof_t *woof   = malloc (sizeof (woof_t));
    state_t *state = malloc (sizeof (state_t));

    woof->state = state;

    // TODO: actually implement if (wayland)
    if (WAYLAND)
        {
            woof->state->wlc        = wlc_init ();
            woof->state->wlc->state = state; // migraine inducing statement
            woof->start             = wlc_start;
            woof->main_loop         = wlc_main_loop;
            woof->cleanup           = wlc_disconnect;
        }
    else if (X11)
        {
            /* setup for x11 would be as follows:
             * woof->state->xc            = xc_init ();
             * woof->state->xc->state    = state; // migraine inducing statement
             * woof->start                = xc_start;
             * woof->main_loop            = xc_main_loop;
             * woof->cleanup              = xc_disconnect;
             */
            die ("uh wait");
        }
    else
        die ("no display server ? :(");

    INFO ("woof initiated :3");
    return woof;
}

void
destroy_woof (woof_t *woof)
{
    woof->cleanup (woof->state);
    free (woof);
}
