#include "render.h"
#include "config.h"
#include "state.h"
#include "util.h"
#include <fontconfig/fontconfig.h>
#include <stdint.h>
#include <stdio.h>
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
    };

bool r_char = false;
void draw_borders (buffer_t *context);

void
blend (const buffer_t *context, const buffer_t *input_buf)
{

    uint32_t *buf          = context->buffer;
    buf                   += (context->width * input_buf->y) + input_buf->x;
    uint32_t *input        = input_buf->buffer;
    uint32_t *fg           = input;
    uint32_t *bg           = buf;
    uint8_t (*(*fg_a))[4]  = (void *)&fg;
    uint8_t (*(*bl_a))[4]  = (void *)&bg;

    uint8_t bg_alpha;
    for (int i = 0; i < input_buf->height; ++i, bg += (context->width - input_buf->width))
        for (int j = 0; j < input_buf->width; ++j, fg++, bg++)
            {
                bg_alpha = (*(*bl_a))[3];

                if ((*(*fg_a))[3] == 0xFF)
                    {
                        *bg = *fg;
                        continue;
                    }

                (*(*bl_a))[3] = (*(*fg_a))[3] + (bg_alpha * (0xFF - (*(*fg_a))[3]) / 0xFF);

                if (!(*(*bl_a))[3])
                    {
                        *bg = 0x0;
                        continue;
                    }

                // left to be individual because gcc will optimize out a loop and it makes more sense like this
                // r
                (*(*bl_a))[2]
                    = (((*(*fg_a))[2] * (*(*fg_a))[3] + (*(*bl_a))[2] * bg_alpha * (0xFF - (*(*fg_a))[3]) / 0xFF)
                       / (*(*bl_a))[3]);
                // g
                (*(*bl_a))[1]
                    = (((*(*fg_a))[1] * (*(*fg_a))[3] + (*(*bl_a))[1] * bg_alpha * (0xFF - (*(*fg_a))[3]) / 0xFF)
                       / (*(*bl_a))[3]);
                // b
                (*(*bl_a))[0]
                    = (((*(*fg_a))[0] * (*(*fg_a))[3] + (*(*bl_a))[0] * bg_alpha * (0xFF - (*(*fg_a))[3]) / 0xFF)
                       / (*(*bl_a))[3]);
            }
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

    blend (context, input_buf);
}

void
draw_color_square (buffer_t *context, uint32_t color, uint32_t width, uint32_t height, int32_t x, int32_t y)
{
    buffer_t temp_buf = INIT_BUF (context, width, height, x, y, temp_buf);
    uint32_t *buf     = temp_buf.buffer;

    for (int i = 0; i < width * height; i++)
        *buf++ = color;
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
draw_cur (buffer_t *context, int x, int y)
{
    draw_color_square (context, 0xFFEBDBB2, 2, 18, x, y);
    // could simply invert the buffer colors with some sorta bit masking ?
}

void
draw_character (SFT *sft, char *string_buf, SFT_UChar chr, uint32_t width, uint32_t height, int32_t *x, int32_t *y)
{
    SFT_Glyph gid;
    if (sft_lookup (sft, chr, &gid) < 0)
        die ("missing glyph owo");

    SFT_GMetrics mtx;
    if (sft_gmetrics (sft, gid, &mtx) < 0)
        die ("bad glyph metrics");

    SFT_Image img = {
        .width  = (mtx.minWidth + 3) & ~3,
        .height = mtx.minHeight,
    };

    char pixels[img.width * img.height];
    img.pixels = pixels;

    if (sft_render (sft, gid, img) < 0)
        die ("failed to render glyph :o");

    *x += mtx.leftSideBearing;

    char *buf = &string_buf[(*y + (height / 2 + mtx.yOffset)) * width];

    // this placement does not account for overflowing the visual bounds of the input buffer
    for (int i = 0; i < img.height; ++i)
        memcpy (&buf[(i * width) + *x], &pixels[i * img.width], img.width);

    *x += mtx.advanceWidth;
}

void
draw_str (buffer_t *context, char *str, int cur)
{
    int32_t x, y, cur_x, cur_y;
    x = PADDING;
    y = (context->height - 18) / 2;

    char *string_buf      = calloc (1, context->width * context->height);
    char *string_buf_temp = string_buf;

    for (int i = 0; i < strlen (str); i++)
        {
            draw_character (context->render_context->sft, string_buf, str[i], context->width, context->height, &x, &y);
            if (cur && i == (int)cur - 1)
                cur_y = x, cur_y = y;
        }
    // convert the char* from sft to uint32_t*
    buffer_t render_buf       = INIT_BUF (context, context->width, context->height, 0, 0, render_buf);
    uint32_t *render_buf_temp = render_buf.buffer;
    for (int i = 0; i < context->width * context->height; ++i)
        *render_buf_temp++ = *string_buf_temp++ << 24 | 0xEBDBB2;

    draw_cur (&render_buf, x, y);
    free (string_buf);
    draw_to_buffer (context, &render_buf);
    free (render_buf.buffer);
}

void
draw_command_str (buffer_t *context)
{
    char *str = context->render_context->state->current_command_string;
    int cur   = context->render_context->state->cursor;

    draw_str (context, str, cur);
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

void
get_font (SFT *sft)
{
    sft->xScale = 18;
    sft->yScale = 18;
    sft->flags  = SFT_DOWNWARD_Y;

    // get font file dir
    FcConfig *config = FcInitLoadConfigAndFonts ();
    // not working rn ??
    FcPattern *pat = FcNameParse ((const FcChar8 *)("Sarasa Term J Nerd Font"));
    FcConfigSubstitute (config, pat, FcMatchPattern);
    FcDefaultSubstitute (pat);

    FcResult res;
    FcPattern *font = FcFontMatch (config, pat, &res);

    char fontFile[256];

    if (font)
        {
            FcChar8 *file = NULL;
            if (FcPatternGetString (font, FC_FILE, 0, &file) == FcResultMatch)
                sprintf (fontFile, "%s", (char *)file);
            FcPatternDestroy (font);
        }
    FcPatternDestroy (pat);

    INFO ("found font: %s", fontFile);

    sft->font = sft_loadfile (fontFile);

    if (sft->font == NULL)
        die ("font failed to load :O");
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

    get_font (sft);

    context->sft = sft;

    return context;
}
