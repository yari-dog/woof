#include "util.h"
#include "wayland/wayland.h"
#include "woof.h"
#include <wayland-client-core.h>
#include <wayland-client.h>

int
main ()
{
    woof_t *woof = init_woof ();
    wlc_start (woof->state->wlc);

    // main loop. entry etc etc etc
    while (!*woof->state->close)
        {
            woof->main_loop (woof->state);
        }

    // function that handles shutting down gracefully
    woof->cleanup (woof->state);
}
