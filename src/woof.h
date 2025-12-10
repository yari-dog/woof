#ifndef WOOF_H
#define WOOF_H
#include "state.h"

typedef struct woof_t
{
    void (*start) (state_t *);
    void (*main_loop) (state_t *);
    void (*cleanup) (state_t *);
    struct state_t *state;
} woof_t;

extern woof_t *g_woof;

woof_t *init_woof ();

void destroy_woof (woof_t *woof);
#endif
