#ifndef XKB_H
#define XKB_H
#include "state.h"
#include <stdint.h>
#include <uchar.h>
#include <xkbcommon/xkbcommon.h>

typedef struct xkb_t
{
    // xkb info
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *state;

    // info for handling qq for quit
    int64_t time_of_last_key;
    char32_t last_key;
} xkb_t;

void xkb_set_keymap (xkb_t *xkb, char *keymap_str);
void xkb_set_state (xkb_t *xkb);
void xkb_handle_key (state_t *state, uint32_t key);
xkb_t *xkb_init ();

#endif
