#include "woof.h"
#include "util.h"
#include "wayland.h"
#include <wayland-client.h>

woof_t *init_woof() {
  INFO("woof initiating :o");
  woof_t *woof = malloc(sizeof(woof_t));
  woof->wlc = wlc_init();
  INFO("woof initiated :3");
  return woof;
}

void destroy_woof(woof_t *woof) { free(woof); }
