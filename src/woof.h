#ifndef WOOF_H
#define WOOF_H
#include "wayland.h"
#include <wayland-client.h>

typedef struct woof_t {
  struct wlc_t *wlc;
} woof_t;

woof_t *init_woof();
void destroy_woof(woof_t *woof);

#endif
