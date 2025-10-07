#include "render.h"
#include "config.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <string.h>

#define INIT_BUF(RENDER_CONTEXT, T_WIDTH, T_HEIGHT, T_X, T_Y)                                                          \
    {                                                                                                                  \
        .width   = T_WIDTH,                                                                                            \
        .height  = T_HEIGHT,                                                                                           \
        .stride  = RENDER_CONTEXT->color_depth * T_WIDTH,                                                              \
        .x       = T_X,                                                                                                \
        .y       = T_Y,                                                                                                \
        .buffer  = calloc (1, T_HEIGHT * RENDER_CONTEXT->color_depth * T_WIDTH),                                       \
        .context = RENDER_CONTEXT,                                                                                     \
    }

void draw_borders (buffer_t *context);

// TODO: make this work with negative positions ? idk.
void
trim_buf (buffer_t *context, buffer_t *input_buf)
{
    int32_t trim_width  = MIN ((int)context->width - input_buf->x, (int)input_buf->width);
    int32_t trim_height = MIN ((int)context->height - input_buf->y, (int)input_buf->height);

    if (MIN (0, trim_width) || MIN (0, trim_height))
        {
            input_buf->height = 0;
            input_buf->width  = 0;
            return;
        }

    // this avoids a div/0 error
    uint32_t stride     = context->stride;
    int32_t trim_stride = context->context->color_depth * trim_width;

    uint32_t *temp_buf = calloc (1, trim_height * trim_stride);
    for (int i = 0; i < trim_height; ++i)
        memcpy (&temp_buf[(i * trim_width)], &input_buf->buffer[(i * input_buf->width)], trim_stride);

    free (input_buf->buffer);

    input_buf->buffer = temp_buf;
    input_buf->width  = trim_width;
    input_buf->height = trim_height;
}

void
draw_to_buffer (buffer_t *context, buffer_t *input_buf)
{
    // will it go off the edge ?
    // if so, trim it up :3
    trim_buf (context, input_buf);

    if (input_buf->height == 0 || input_buf->height == 0)
        return;

    uint32_t *buf  = context->buffer;
    buf           += context->width * input_buf->y;
    int count      = 0;
    for (int i = 0; i < input_buf->height; ++i)
        for (int j = 0; j < input_buf->width; ++j)
            {
                // i could use memcpy, but i want to be able to deal with transparency (for text, etc)
                buf[(i * context->width) + j + input_buf->x] = input_buf->buffer[(i * input_buf->width) + j];
                // INFO ("%i", ++count);
            }
}

void
draw_checkerboard (buffer_t *context, uint32_t bg, uint32_t color, uint32_t width, uint32_t height, int32_t x,
                   int32_t y)
{
    buffer_t temp_buf = INIT_BUF (context->context, width, height, x, y);
    for (int i = 0; i < height; ++i)
        for (int j = 0; j < width; ++j)
            {
                if ((i + j / 8 * 8) % 16 < 8)
                    temp_buf.buffer[i * width + j] = color;
                else
                    temp_buf.buffer[i * width + j] = bg;
            }

    // draw that to the surface buffer
    draw_to_buffer (context, &temp_buf);
    free (temp_buf.buffer);
}

void
draw_color_square (buffer_t *context, uint32_t color, uint32_t width, uint32_t height, int32_t x, int32_t y)
{
    buffer_t temp_buf = INIT_BUF (context->context, width, height, x, y);

    for (int i = 0; i < width * height; i++)
        temp_buf.buffer[i] = color;
    draw_to_buffer (context, &temp_buf);
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
draw_input (buffer_t *context)
{
    draw_color_square (context, COLOR_BG, WIDTH, COMMAND_HEIGHT, 0, HEIGHT - COMMAND_HEIGHT);
    // TODO: command draw
}

void
draw_results (buffer_t *context)
{
    buffer_t temp_buf = INIT_BUF (context->context, WIDTH, (HEIGHT - PADDING - COMMAND_HEIGHT), 0, 0);
    draw_color_square (&temp_buf, COLOR_BG, temp_buf.width, temp_buf.height, 0, 0);

    draw_borders (&temp_buf);
    draw_to_buffer (context, &temp_buf);
    free (temp_buf.buffer);
    // TODO: draw results
}

void
draw_borders (buffer_t *context)
{
    if (!BORDER_WIDTH)
        return;

    draw_color_square (context, BORDER_COLOR, context->width, BORDER_WIDTH, 0, 0);  // top
    draw_color_square (context, BORDER_COLOR, BORDER_WIDTH, context->height, 0, 0); // left
    draw_color_square (context, BORDER_COLOR, context->width, BORDER_WIDTH, 0,
                       context->height - BORDER_WIDTH); // bottom
    draw_color_square (context, BORDER_COLOR, BORDER_WIDTH, context->height, context->width - BORDER_WIDTH, 0); // right
}
void
render (render_context_t *context)
{
    INFO ("rendering into buffer :3");
    draw_main_surface (context);
    draw_input (context->surface_buf);
    draw_results (context->surface_buf);
    // some tests
#ifdef RENDER_TESTING
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
#endif
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
