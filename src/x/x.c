#include "x.h"
#include "../config.h"
#include "../render.h"
#include "../state.h"
#include "../util.h"
#include "../xkb.h" // i have all wayland listener logic in the listeners file.
                    //

// TODO: (x): keyboard
// you need to do xkb_set_keymap, xkb_handle_key and xkb_state_update_mask
// there is going to be additional functions for shit like key repeat etc, but i havent implemented that in wayland yet.

// TODO: (x): init
// build the struct and do general setup shit
void // type should be the xc_t struct that you make
xc_init ()
{
}

// TODO: (x): start
// this is where you do the actual x logic to start shit up. once this returns, the main loop will start.
void
xc_start (state_t *state)
{
}

// TODO: (x): main loop
// this needs to check if state->update is true, and if it is, run render(), and then swap your buffer, then do whatever
// x shit you need to do to draw the buffer to the screen.
// you can see the buffer swap in wlc_set_surface and wlc_resize_handler
// set_surface's double buffer is the swap that occurs when rendering a buffer to the screen
// resize handler is just the code that makes the buffer. it makes buf1, swaps the buffers, and makes buf2
// render() handles swapping the render buffers. you just need to swap which one is being rendered on a frame update.
void
xc_main_loop (state_t *state)
{
}

// TODO: (x): disconnect
// just whatever you need to do on close.
// gets called when state->close is true.
void
xc_disconnect (state_t *state)
{
}
