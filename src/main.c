#include "woof.h"

woof_t *g_woof;

int
main ()
{
    // build initial data structure for woof, including abstracting windowing server
    g_woof = init_woof ();
    g_woof->start (g_woof->state);

    int64_t start_time = ms_since_epoch ();

    // main loop. entry etc etc etc
    while (!g_woof->state->close)
        if (ms_since_epoch () - start_time < 20) // you don't need more than 50fps on a menu
            {
                g_woof->main_loop (g_woof->state);
                start_time = ms_since_epoch ();
            }

    // function that handles shutting down gracefully
    destroy_woof (g_woof);
}
