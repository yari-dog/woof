#ifndef STATE_H
#define STATE_H
#include "wayland/wayland.h"

typedef struct keys_t
{
    char *current_input;
} keys_t;

typedef struct state_t
{
    struct wlc_t *wlc;
    struct keys_t *keys;
    char *title;
    bool close;
    uint32_t width;
    uint32_t height;
    uint32_t stride; // how many bytes is there in a line
} state_t;

#endif
