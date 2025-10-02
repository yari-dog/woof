#ifndef STATE_H
#define STATE_H
#include "wayland/wayland.h"

typedef struct state_t
{
    struct wlc_t *wlc;
    char *current_input;
    char *title;
    bool close;
    uint32_t width;
    uint32_t height;
    uint32_t stride; // how many bytes is there in a line
} state_t;

#endif
