#include "render.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <string.h>
#include <wayland-client-protocol.h>

void
draw_square (render_context_t *context, uint32_t *surface_buf, uint32_t *input_buf, uint32_t stride, uint32_t width,
             uint32_t height, uint32_t x, uint32_t y)
{
    // will it go off the edge ?
    if (x + width > context->width)
        {
            INFO ("too fuckin wide mate x%i w%i sw%i", x, width, context->width);
            return;
        }

    if (y + height > context->height)
        {
            INFO ("too fuckin tall mate y%i h%i sh%i", y, height, context->height);
            return;
        }

    surface_buf += context->width * y;
    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j)
            surface_buf[(i * context->width) + j + x] = input_buf[(i * width) + j];

    // memcpy (&surface_buf[(i * context->width) + x], &input_buf[(i * width)], stride);
}

void
draw_checkerboard (render_context_t *context, uint32_t *surface_buffer, uint32_t bg, uint32_t color, uint32_t width,
                   uint32_t height, uint32_t x, uint32_t y)
{
    uint32_t stride    = context->color_depth * width;
    uint32_t *temp_buf = malloc (stride * height);

    for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
                {
                    if ((i + j / 8 * 8) % 16 < 8)
                        temp_buf[i * width + j] = color;
                    else
                        temp_buf[i * width + j] = bg;
                }
        }

    // draw that to the surface buffer
    draw_square (context, surface_buffer, temp_buf, stride, width, height, x, y);
    free (temp_buf);
}

// TODO: make this work
void
draw_color_square (render_context_t *context, uint32_t *surface_buf, int32_t color, uint32_t width, uint32_t height,
                   uint32_t x, uint32_t y)
{
    uint32_t stride    = context->color_depth * width;
    uint32_t *temp_buf = malloc (stride * height);
    for (int i = 0; i < width * height; i++)
        temp_buf[i] = color;
    draw_square (context, surface_buf, temp_buf, stride, width, height, x, y);
    free (temp_buf);
}

// TODO: also make this work. i think that im fuckin shit up somewhere
void
draw_main_surface (render_context_t *context, uint32_t *surface_buf)
{
    // set surface to be transparent first.
    memset (surface_buf, 0x00, context->stride * context->height);
    draw_color_square (context, surface_buf, 0x1a2b3c4d, context->width, context->height, 0, 0);
}

void
render (render_context_t *context, uint32_t *surface_buf)
{
    INFO ("rendering into buffer :3");
    draw_main_surface (context, surface_buf);
    draw_color_square (context, surface_buf, 0xFF282828, 80, 80, 0, 0);
    draw_color_square (context, surface_buf, 0xFFEBDBB2, 80, 80, 100, 100);
    draw_checkerboard (context, surface_buf, 0xFFEBDBB2, 0xFF282828, 80, 80, 0, 100);
    draw_checkerboard (context, surface_buf, 0xFF282828, 0xFFEBDBB2, 80, 80, 100, 0);
    INFO ("rendered into buffer ");
}

render_context_t *
render_init (state_t *state)
{
    render_context_t *context = calloc (1, sizeof (render_context_t));
    context->color_depth      = 4; // WL_SHM_FORMAT_XRGB8888 = 32 bit = 4

    return context;
}
