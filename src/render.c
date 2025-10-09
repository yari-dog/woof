#include "render.h"
#include "config.h"
#include "state.h"
#include "util.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INIT_BUF(CONTEXT, T_WIDTH, T_HEIGHT, T_X, T_Y, T_NAME)                                                         \
    {                                                                                                                  \
        .width          = T_WIDTH,                                                                                     \
        .height         = T_HEIGHT,                                                                                    \
        .stride         = CONTEXT->render_context->color_depth * T_WIDTH,                                              \
        .x              = T_X,                                                                                         \
        .y              = T_Y,                                                                                         \
        .global_x       = CONTEXT->global_x + T_X,                                                                     \
        .global_y       = CONTEXT->global_y + T_Y,                                                                     \
        .buffer         = calloc (1, T_HEIGHT * CONTEXT->render_context->color_depth * T_WIDTH),                       \
        .render_context = CONTEXT->render_context,                                                                     \
    };                                                                                                                 \
    for (int i = 0; i < T_HEIGHT; ++i)                                                                                 \
        memcpy (&T_NAME.buffer[(i * T_WIDTH)], &CONTEXT->buffer[(i * CONTEXT->width) + T_X], T_NAME.stride);

void draw_borders (buffer_t *context);

uint32_t
blend (uint32_t bg, uint32_t fg)
{
    if (fg >> 24 == 0xFF)
        return fg;

    uint32_t blend;

    uint32_t bg_a, bg_r, bg_g, bg_b;
    uint32_t fg_a, fg_r, fg_g, fg_b;
    uint32_t b_a, b_r, b_g, b_b;

    bg_a = (bg & 0xFF000000) >> 24;
    fg_a = (fg & 0xFF000000) >> 24;
    b_a  = fg_a + (bg_a * (0xFF - fg_a) / 0xFF);

    if (b_a <= 0x0)
        return 0x0;

    bg_r = (bg & 0xFF0000) >> 16;
    bg_g = (bg & 0xFF00) >> 8;
    bg_b = (bg & 0xFF);

    fg_r = (fg & 0xFF0000) >> 16;
    fg_g = (fg & 0xFF00) >> 8;
    fg_b = (fg & 0xFF);

    b_r = ((fg_r * fg_a + bg_r * bg_a * (0xFF - fg_a) / 0xFF) / b_a) << 16;
    b_g = ((fg_g * fg_a + bg_g * bg_a * (0xFF - fg_a) / 0xFF) / b_a) << 8;
    b_b = ((fg_b * fg_a + bg_b * bg_a * (0xFF - fg_a) / 0xFF) / b_a);
    b_a = b_a << 24;

    blend = b_a | b_r | b_g | b_b;
    return blend;
}

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
    int32_t trim_stride = context->render_context->color_depth * trim_width;

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

    uint32_t blended_color;
    uint32_t *buf  = context->buffer;
    buf           += context->width * input_buf->y;
    int count      = 0;
    for (int i = 0; i < input_buf->height; ++i)
        for (int j = 0; j < input_buf->width; ++j)
            {
                // if (input_buf->buffer[(i * input_buf->width) + j] != 0x0)
                // buf[(i * context->width) + j + input_buf->x] = input_buf->buffer[(i * input_buf->width) + j];
                blended_color = blend (buf[(i * context->width) + j + input_buf->x],
                                       input_buf->buffer[(i * input_buf->width) + j]);

                buf[(i * context->width) + j + input_buf->x] = blended_color;
            }
}

void
draw_checkerboard (buffer_t *context, uint32_t bg, uint32_t color, uint32_t width, uint32_t height, int32_t x,
                   int32_t y)
{
    buffer_t temp_buf = INIT_BUF (context, width, height, x, y, temp_buf);
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
    buffer_t temp_buf = INIT_BUF (context, width, height, x, y, temp_buf);

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

// move 0x------ff to 0xff------
uint32_t
font_mask (char chr)
{
    if (chr == 0x0)
        return 0x0;

    return chr << 24 | 0xEBDBB2;
}

void
draw_character (buffer_t *context, SFT_UChar chr, int32_t *x, int32_t *y)
{
    SFT_Glyph gid;
    if (sft_lookup (context->render_context->sft, chr, &gid) < 0)
        die ("missing glyph owo");

    SFT_GMetrics mtx;
    if (sft_gmetrics (context->render_context->sft, gid, &mtx) < 0)
        die ("bad glyph metrics");

    SFT_Image img = {
        .width  = (mtx.minWidth + 3) & ~3,
        .height = mtx.minHeight,
    };

    char pixels[img.width * img.height];
    img.pixels = pixels;

    if (sft_render (context->render_context->sft, gid, img) < 0)
        die ("failed to render glyph :o");

    *x += mtx.leftSideBearing;

    buffer_t temp_buf
        = INIT_BUF (context, img.width, img.height, *x, *y + (context->height / 2 + mtx.yOffset), temp_buf);

    for (int i = 0; i < img.width * img.height; i++)
        temp_buf.buffer[i] = font_mask (pixels[i]);

    *x += mtx.advanceWidth;
    draw_to_buffer (context, &temp_buf);
}

void
draw_cur (buffer_t *context, int x, int y)
{
    draw_color_square (context, 0xFFEBDBB2, 2, 18, x, y);
}
void
draw_command_str (buffer_t *context)
{
    // for (char *chr = context->render_context->state->current_command_string; *chr != '\0'; chr++)
    //     draw_character (context, *chr);
    char *str = context->render_context->state->current_command_string;
    int cur   = context->render_context->state->cursor;

    int32_t x, y;

    x = PADDING;
    y = (context->height - 18) / 2;

    for (int i = 0; i < strlen (str); i++)
        {
            draw_character (context, str[i], &x, &y);
            if (i == (int)cur - 1)
                draw_cur (context, x, y);
        }
    //     draw_color_square (context, (int)cur - 1 == i ? COLOR_BG : COLOR_FG, COMMAND_HEIGHT, COMMAND_HEIGHT,
    //                        PADDING + (i * COMMAND_HEIGHT), 0);
}

void
draw_input (buffer_t *context)
{
    buffer_t temp_buf = INIT_BUF (context, WIDTH, COMMAND_HEIGHT, 0, HEIGHT - COMMAND_HEIGHT, temp_buf);
    draw_color_square (&temp_buf, COMMAND_BG, temp_buf.width, temp_buf.height, 0, 0);
    // TODO: command draw

    draw_command_str (&temp_buf);

    draw_to_buffer (context, &temp_buf);
    free (temp_buf.buffer);
}

void
draw_results (buffer_t *context)
{
    buffer_t temp_buf = INIT_BUF (context, WIDTH, (HEIGHT - PADDING - COMMAND_HEIGHT), 0, 0, temp_buf);
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
    context->state->update = false;
    INFO ("rendered into buffer ");
}

render_context_t *
render_init (state_t *state)
{
    render_context_t *context   = calloc (1, sizeof (render_context_t));
    buffer_t *surface_buf       = calloc (1, sizeof (buffer_t));
    context->state              = state;
    context->color_depth        = COLOR_DEPTH;
    context->surface_buf        = surface_buf;
    surface_buf->height         = HEIGHT;
    surface_buf->width          = WIDTH;
    surface_buf->x              = 0;
    surface_buf->y              = 0;
    surface_buf->render_context = context;

    // font shit
    SFT *sft = calloc (1, sizeof (SFT));

    sft->xScale = 18;
    sft->yScale = 18;
    sft->flags  = SFT_DOWNWARD_Y;

    sft->font = sft_loadfile ("/usr/share/fonts/TTF/IosevkaTermNerdFontMono-Regular.ttf");
    // sft->font = sft_loadfile ("/home/yari/dev/woof/src/FiraGO-Regular.ttf");

    if (sft->font == NULL)
        die ("font failed to load :O");

    context->sft = sft;

    return context;
}
