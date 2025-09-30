#include "util.h"
#include "wayland/wayland.h"
#include "woof.h"
#include <wayland-client.h>

int main() {
    woof_t *woof = init_woof();

    wlc_start(woof->wlc);
    INFO("close %s? %u", woof->wlc->title, woof->wlc->close);

    bool redraw = true;

    // main loop. entry etc etc etc
    while (!woof->wlc->close) {
        if (redraw) {
            draw_frame(woof->wlc);
            redraw = false;
        }
    }

    // function that handles shutting down gracefully
    wlc_disconnect(woof->wlc);
}
