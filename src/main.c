#include "util.h"
#include "wayland/wayland.h"
#include "woof.h"
#include <wayland-client-core.h>
#include <wayland-client.h>

int
main ()
{
    woof_t *woof = init_woof ();

    wlc_start (woof->wlc);
    INFO ("close %s? %u", woof->wlc->title, woof->wlc->close);

    // main loop. entry etc etc etc
    while (!woof->wlc->close)
        {
            wl_display_flush (woof->wlc->display);

            if (!wl_display_dispatch_pending (woof->wlc->display))
                if (wl_display_dispatch (woof->wlc->display) < 0)
                    die ("display dispatch failed :(");
        }

    // function that handles shutting down gracefully
    wlc_disconnect (woof->wlc);
}
