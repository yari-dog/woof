#include "wayland.h"
#include "woof.h"
#include <wayland-client.h>

int main() {
  woof_t woof;
  woof_init(&woof);

  while (1)
    wl_display_dispatch(woof.wlc->display);

  wlc_disconnect(woof.wlc);
}
