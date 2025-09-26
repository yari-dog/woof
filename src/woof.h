#ifndef WOOF_H
#define WOOF_H
#include "wayland.h"
#include <wayland-client.h>

typedef struct woof_t woof_t;

struct woof_t {
  struct wlc_t *wlc;
};

void woof_init(woof_t *woof);

#endif
