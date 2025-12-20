#include "util.h"
#include "woof.h"
#include <stdint.h>

woof_t *g_woof;

int
main ()
{
    // build initial data structure for woof, including abstracting windowing server
    g_woof = init_woof ();
    g_woof->start (g_woof->state);

    uint64_t current_time;
    // main loop. entry etc etc etc
    while (!g_woof->state->close)
        {
            current_time = ms_since_epoch ();
            if (current_time - g_woof->state->run_time > 20) // you don't need more than 50fps on a menu
                {
                    if (g_woof->state->typing && current_time - g_woof->state->time_since_typing > 200)
                        g_woof->state->typing = false;
                    if (current_time - g_woof->state->cur_time > 1000 && !g_woof->state->typing)
                        {
                            g_woof->state->cur_visible = !g_woof->state->cur_visible;
                            g_woof->state->update      = true;
                        }
                    g_woof->main_loop (g_woof->state);
                    g_woof->state->run_time = current_time;
                    if (current_time - g_woof->state->cur_time > 1000)
                        g_woof->state->cur_time = current_time;
                }
        }

    // function that handles shutting down gracefully
    destroy_woof (g_woof);
}
