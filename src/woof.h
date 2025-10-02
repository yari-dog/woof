#ifndef WOOF_H
#define WOOF_H
#include "state.h"
#include "wayland/wayland.h"

typedef struct woof_t
{
    void (*main_loop) (state_t *);
    void (*cleanup) (state_t *);
    struct state_t *state;
} woof_t;

woof_t *init_woof ();
void destroy_woof (woof_t *woof);

#endif
