#ifndef STATE_H
#define STATE_H
#include "wayland/wayland.h"

typedef struct state_t
{
    struct wlc_t *wlc;
    bool *close;
} state_t;

#endif
