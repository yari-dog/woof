#ifndef RENDER_H
#define RENDER_H
#include "state.h"
#include <stdint.h>
#define COLORDEPTHSLUDGE 4

typedef struct buffer_t buffer_t;

typedef struct render_context_t
{
    uint8_t color_depth;
    buffer_t *surface_buf;
    state_t *state;
} render_context_t;

typedef struct buffer_t
{
    uint32_t stride;
    uint32_t *buffer;
    uint32_t width;
    uint32_t height;
    render_context_t *context;
} buffer_t;

void render (render_context_t *context);
render_context_t *render_init (state_t *state);

#endif
