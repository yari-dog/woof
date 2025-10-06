#ifndef RENDER_H
#define RENDER_H
#include "state.h"
#include <stdint.h>

#define COLORDEPTHSLUDGE 4
typedef struct render_context_t
{
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint8_t color_depth;
    state_t *state;
} render_context_t;

void render (render_context_t *context, uint32_t *buffer);
render_context_t *render_init (state_t *state);

#endif
