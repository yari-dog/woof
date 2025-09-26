#include "woof.h"
#include "wayland.h"
#include <stdio.h>
#include <wayland-client.h>

void woof_init(woof_t *woof) {
  printf("(INFO) woof_init\n");

  // create wayland stuff
  wlc_t wlc;
  wlc.title = ":woof";
  woof->wlc = &wlc;
  wlc_init(woof->wlc);
}
