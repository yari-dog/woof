#ifndef STATE_H
#define STATE_H
#include "wayland/wayland.h"
#include "xkb.h"

typedef struct state_t
{
    // window servers
    struct wlc_t *wlc;

    // key handling
    struct xkb_t *xkb;

    // various window server agnostic bits
    char *title;
    char *current_command_string;
    bool close;
    uint32_t width;
    uint32_t height;
    uint32_t stride; // how many bytes is there in a line
} state_t;

#endif
