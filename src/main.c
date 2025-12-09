#include "util.h"
#include "woof.h"

int
main ()
{
    // build initial data structure for woof, including abstracting windowing server
    woof_t *woof = init_woof ();
    woof->start (woof->state);

    int64_t start_time = ms_since_epoch ();

    // main loop. entry etc etc etc
    while (!woof->state->close)
        if (ms_since_epoch () - start_time < 100) // you don't need more than 10fps on a menu
            {
                woof->main_loop (woof->state);
                start_time = ms_since_epoch ();
            }

    // function that handles shutting down gracefully
    destroy_woof (woof);
}
