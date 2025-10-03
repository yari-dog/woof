#ifndef XKB_H
#define XKB_H
#include <xkbcommon/xkbcommon.h>

typedef struct xkb_t
{
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
} xkb_t;

void xkb_set_keymap (xkb_t *xkb, char *keymap_str);
xkb_t *xkb_init ();

#endif
