#ifndef STATE_H
#define STATE_H
#include "config.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct state_t
{
    // window servers
    struct wlc_t *wlc;

    // key handling
    struct xkb_t *xkb;
    size_t cursor;

    // various window server agnostic bits
    struct render_context_t *render_context;
    char *title;
    char *current_command_string;
    int32_t ccs_buffsize;
    bool close;
    bool update;
} state_t;

state_t *init_state ();

#endif
