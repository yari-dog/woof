#ifndef STATE_H
#define STATE_H
#include "wayland/wayland.h"

typedef struct state_t
{
    struct wlc_t *wlc;
    char *current_input;
    bool *close;
} state_t;

#endif
