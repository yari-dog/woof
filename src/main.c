#include "woof.h"

int
main ()
{
    // build initial data structure for woof, including abstracting windowing server
    woof_t *woof = init_woof ();
    woof->start (woof->state);

    // main loop. entry etc etc etc
    while (!woof->state->close)
        {
            woof->main_loop (woof->state);
        }

    // function that handles shutting down gracefully
    destroy_woof (woof);
}
