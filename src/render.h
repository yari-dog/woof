#ifndef RENDER_H
#define RENDER_H
#include "state.h"
#include <stdint.h>

#define COLORDEPTHSLUDGE 4
typedef struct render_context_t {
    int32_t width;
    int32_t height;
    int32_t stride;
    int32_t color_depth;
    state_t *state;
} render_context_t;

void render (render_context_t *context, uint32_t *buffer);
render_context_t *render_init (state_t *state);

#endif
