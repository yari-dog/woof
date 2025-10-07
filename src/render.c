#include "render.h"
#include "config.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <string.h>

#define INIT_BUF(T_WIDTH, T_HEIGHT, T_STRIDE)                                                                          \
    {                                                                                                                  \
        .width  = T_WIDTH,                                                                                             \
        .height = T_HEIGHT,                                                                                            \
        .stride = T_STRIDE,                                                                                            \
        .buffer = calloc (1, T_WIDTH * T_STRIDE),                                                                      \
    }

// TODO: make this work with negative positions ? idk.
void
trim_buf (buffer_t *context, buffer_t *input_buf, int32_t x, int32_t y)
{
    INFO ("trimming, %i %i %i %i %i %i", context->height, context->width, input_buf->height, input_buf->width, x, y);
    int32_t trim_width  = MIN ((int)context->width - x, (int)input_buf->width);
    int32_t trim_height = MIN ((int)context->height - y, (int)input_buf->height);

    if (MIN (0, trim_width) || MIN (0, trim_height))
        {
            input_buf->height = 0;
            input_buf->width  = 0;
            return;
        }

    // this avoids a div/0 error
    uint32_t stride     = context->stride;
    int32_t trim_stride = stride / (input_buf->width + !input_buf->width) * (trim_width + !trim_width);

    uint32_t *temp_buf = calloc (1, trim_width * trim_stride);
    for (int i = 0; i < trim_height; ++i)
        memcpy (&temp_buf[(i * trim_width)], &input_buf->buffer[(i * input_buf->width)], trim_stride);

    free (input_buf->buffer);

    input_buf->buffer = temp_buf;
    input_buf->width  = trim_width;
    input_buf->height = trim_height;
    INFO ("trimmed %i %i", trim_width, trim_height);
}

void
draw_to_buffer (buffer_t *context, buffer_t *input_buf, int32_t x, int32_t y)
{
    INFO ("drawing to buffer");
    // will it go off the edge ?
    // if so, trim it up :3
    trim_buf (context, input_buf, x, y);

    if (input_buf->height == 0 || input_buf->height == 0)
        return;

    uint32_t *buf  = context->buffer;
    buf           += context->width * y;
    for (int i = 0; i < input_buf->height; ++i)
        for (int j = 0; j < input_buf->width; ++j)
            // i could use memcpy, but i want to be able to deal with transparency (for text, etc)
            buf[(i * context->width) + j + x] = input_buf->buffer[(i * input_buf->width) + j];
    INFO ("drawn to buffer");
}

void
draw_checkerboard (buffer_t *context, uint32_t bg, uint32_t color, uint32_t width, uint32_t height, int32_t x,
                   int32_t y)
{
    buffer_t temp_buf = INIT_BUF (width, height, context->stride * height);
    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j)
            {
                if ((i + j / 8 * 8) % 16 < 8)
                    temp_buf.buffer[i * width + j] = color;
                else
                    temp_buf.buffer[i * width + j] = bg;
            }

    // draw that to the surface buffer
    draw_to_buffer (context, &temp_buf, x, y);
    free (temp_buf.buffer);
}

void
draw_color_square (buffer_t *context, uint32_t color, uint32_t width, uint32_t height, int32_t x, int32_t y)
{
    buffer_t temp_buf = INIT_BUF (width, height, context->stride * height);

    for (int i = 0; i < width * height; i++)
        temp_buf.buffer[i] = color;
    draw_to_buffer (context, &temp_buf, x, y);
    free (temp_buf.buffer);
}

// TODO: also make this work. i think that im fuckin shit up somewhere
void
draw_main_surface (render_context_t *context)
{
    // set surface to be transparent first.
    memset (context->surface_buf->buffer, 0x00, context->surface_buf->stride * context->surface_buf->height);
    // draw_color_square (context, surface_buf, 0x1a2b3c4d, context->width, context->height, 0, 0);
}

void
render (render_context_t *context)
{
    INFO ("rendering into buffer :3");
    draw_main_surface (context);
    // some tests
    draw_color_square (context->surface_buf, 0xFF282828, 80, 80, 0, 0);
    draw_color_square (context->surface_buf, 0xFFEBDBB2, 80, 80, 100, 100);
    draw_checkerboard (context->surface_buf, 0xFFEBDBB2, 0xFF282828, 80, 80, 0, 100);
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFEBDBB2, 80, 80, 100, 0);
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFFFFFFF, 80, 80, 760, 0);   // should half-draw
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFFFFFFF, 80, 80, 760, 700); // shouldn't draw at all
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFFFFFFF, 80, 80, 860, 100); // shouldn't draw at all
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFFFFFFF, 80, 80, 70, 360);  // should half-draw
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFFFFFFF, 80, 80, 860, 700); // shouldn't draw at all
    draw_checkerboard (context->surface_buf, 0xFF282828, 0xFFFFFFFF, 80, 80, 760, 360); // should half-draw
    INFO ("rendered into buffer ");
}

render_context_t *
render_init (state_t *state)
{
    render_context_t *context = calloc (1, sizeof (render_context_t));
    buffer_t *surface_buf     = calloc (1, sizeof (buffer_t));
    context->color_depth      = COLOR_DEPTH;
    context->surface_buf      = surface_buf;
    surface_buf->height       = HEIGHT;
    surface_buf->width        = WIDTH;
    surface_buf->context      = context;

    return context;
}
