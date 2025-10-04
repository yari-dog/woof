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

    uint32_t *current_line = &surface_buf[context->width * y];
    for (int i = 0; i < height; i++)
        {
            memcpy (&current_line[x], &input_buf[i * width], stride);
            current_line = &current_line[context->width];
        }
}

void
draw_checkerboard (render_context_t *context, uint32_t *surface_buffer, uint32_t width, uint32_t height, uint32_t x,
                   uint32_t y)
{
    uint32_t stride    = context->color_depth * width;
    uint32_t *temp_buf = malloc (stride * height);

    // print checkerboard to temp buff
    uint32_t bg    = 0xFF282828;
    uint32_t color = 0xFFEBDBB2;
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
draw_color_square (render_context_t *context, uint32_t *surface_buf, uint32_t color, uint32_t width, uint32_t height,
                   uint32_t x, uint32_t y)
{
    uint32_t stride    = context->color_depth * width;
    uint32_t *temp_buf = malloc (stride * height);
    for (int i = 0; i < width * height; i++)
        temp_buf[i] = color;
    draw_square (context, surface_buf, temp_buf, stride, width, height, x, y);
    free (temp_buf);
}
void
draw_main_surface (render_context_t *context, uint32_t *surface_buf)
{
    memset (surface_buf, 0xFFFFFFFF, context->stride * context->height);
    // int height    = context->height;
    // int width     = context->width;
    // int32_t bg    = 0xFF282828;
    // int32_t color = 0xFFEBDBB2;
    // for (int i = 0; i < height; ++i)
    //     {
    //         for (int j = 0; j < width; ++j)
    //             {
    //                 if ((i + j / 8 * 8) % 16 < 8)
    //                     surface_buf[i * width + j] = color;
    //                 // else
    //                 //     surface_buf[i * width + j] = bg;
    //             }
    //     }
}

void
render (render_context_t *context, uint32_t *surface_buf)
{
    INFO ("rendering into buffer :3");
    draw_main_surface (context, surface_buf);
    draw_color_square (context, surface_buf, 0xFF282828, 80, 80, 0, 0);
    draw_color_square (context, surface_buf, 0xFFEBDBB2, 80, 80, 100, 100);
    draw_checkerboard (context, surface_buf, 80, 80, 0, 100);
    draw_checkerboard (context, surface_buf, 80, 80, 100, 0);
    INFO ("rendered into buffer ");
}

render_context_t *
render_init (state_t *state)
{
    render_context_t *context = calloc (1, sizeof (render_context_t));
    context->color_depth      = 4; // WL_SHM_FORMAT_XRGB8888 = 32 bit = 4

    return context;
}
