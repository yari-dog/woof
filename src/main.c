#include "util.h"
#include "wayland.h"
#include "woof.h"
#include <wayland-client.h>

int main() {
  woof_t *woof = init_woof();

  wlc_start(woof->wlc);
  INFO("close %s? %u", woof->wlc->title, woof->wlc->close);

  // main loop. entry etc etc etc
  while (!woof->wlc->close) {
    if (!wl_display_dispatch_pending(woof->wlc->display)) {
      wl_display_dispatch(woof->wlc->display);
    }
  }

  // function that handles shutting down gracefully
  wlc_disconnect(woof->wlc);
}
