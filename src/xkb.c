#include "xkb.h"
#include "state.h"
#include "util.h"
#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>

void
xkb_set_state (xkb_t *xkb)
{
    INFO ("xkb_set_state");
    if (xkb->state)
        xkb_state_unref (xkb->state);

    xkb->state = xkb_state_new (xkb->keymap);
}

void
xkb_set_keymap (xkb_t *xkb, char *keymap_str)
{
    INFO ("xkb_set_keymap");
    if (xkb->keymap)
        xkb_keymap_unref (xkb->keymap);

    xkb->keymap
        = xkb_keymap_new_from_string (xkb->context, keymap_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);

    xkb_set_state (xkb);
}

xkb_t *
xkb_init ()
{
    xkb_t *xkb   = calloc (1, sizeof (xkb_t));
    xkb->context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    return xkb;
}
