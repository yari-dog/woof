#include "render.h"
#include "config.h"
#include "state.h"
#include "util.h"
#include "woof.h"
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

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define A 3
#define R 2
#define G 1
#define B 0
#else
#define A 0
#define R 1
#define G 2
#define B 3
#endif

static void
blend (const buffer_t *context, const buffer_t *input_buf)
{
    uint8_t (*bg)[4] = (void *)(context->buffer + (context->width * input_buf->y) + input_buf->x);
    uint8_t (*fg)[4] = (void *)input_buf->buffer;

    uint8_t inv_alpha;
    for (int i = 0; i < input_buf->height; ++i, bg += (context->width - input_buf->width))
        for (int j = 0; j < input_buf->width; ++j, fg++, bg++)
            {
                // if fg is transparent just skip
                if (!(*fg)[A])
                    continue;

                // if bg is transparent or fg is 100%
                if (!(*bg)[A] || 0xFF == (*fg)[A])
                    {
                        memcpy (bg, fg, sizeof (uint32_t));
                        continue;
                    }

                // TODO: parelellize? https://stackoverflow.com/questions/12011081/alpha-blending-2-rgba-colors-in-c
                // https://www.daniweb.com/programming/software-development/code/216791/alpha-blend-algorithm
                inv_alpha = 255 - (*fg)[A];

                (*bg)[A]  = (*fg)[A] + ((*bg)[A] * inv_alpha >> 8);            // a
                (*bg)[R]  = ((*fg)[A] * (*fg)[R] + inv_alpha * (*bg)[R]) >> 8; // r
                (*bg)[G]  = ((*fg)[A] * (*fg)[G] + inv_alpha * (*bg)[G]) >> 8; // g
                (*bg)[B]  = ((*fg)[A] * (*fg)[B] + inv_alpha * (*bg)[B]) >> 8; // b
            }
}

static void
trim_buf (buffer_t *context, buffer_t *input_buf)
{

    // TODO: make this work with negative x and y
    int32_t trim_width  = MIN ((int)context->width - (int)input_buf->x, (int)input_buf->width);
    int32_t trim_height = MIN ((int)context->height - (int)input_buf->y, (int)input_buf->height);

    // if nothing will be visible, just make it do nothing
    if (MIN (0, trim_width) || MIN (0, trim_height))
        {
            input_buf->height = 0;
            input_buf->width  = 0;
            return;
        }

    if (trim_width >= input_buf->width && trim_height >= input_buf->height)
        return;

    INFO ("trimming");
    uint32_t trim_stride = context->render_context->color_depth * trim_width;

    uint32_t *temp_buf   = calloc (1, trim_height * trim_stride);
    for (int i = 0; i < trim_height; ++i)
        memcpy (&temp_buf[(i * trim_width)], &input_buf->buffer[(i * input_buf->width)], trim_stride);

    free (input_buf->buffer);

    input_buf->buffer = temp_buf;
    input_buf->width  = trim_width;
    input_buf->height = trim_height;
}

// should_blend exists because tbh there's not much point doing some fancy test
// we'll know if something will have < FF alpha (because its text)
// idk how to overload function definitions
static void
draw_to_buffer (buffer_t *context, buffer_t *input_buf, bool should_blend)
{
    // will it go off the edge ?
    trim_buf (context, input_buf);

    if (input_buf->height == 0 || input_buf->height == 0)
        return;

    // call with should_blend=true if transparency is likely
    if (should_blend)
        blend (context, input_buf);
    else
        for (int i = 0; i < input_buf->height; ++i)
            memcpy (&context->buffer[(context->width * (i + input_buf->y)) + input_buf->x],
                    &input_buf->buffer[i * input_buf->width], input_buf->stride);
}

static void
draw_color_square (buffer_t *context, uint32_t color, uint32_t width, uint32_t height, int32_t x, int32_t y)
{
    buffer_t temp_buf = INIT_BUF (context, width, height, x, y, temp_buf);
    uint32_t *buf     = temp_buf.buffer;

    for (int i = 0; i < width * height; i++)
        *buf++ = color;

    draw_to_buffer (context, &temp_buf, width != context->width && height != context->height);
    free (temp_buf.buffer);
}

static void
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

static void
draw_cur (buffer_t *context, int x, int y)
{
    draw_color_square (context, COLOR_FG, 2, 1 * FONT_SCALE, x, y);
    // could simply invert the buffer colors with some sorta bit masking ?
}

static void
draw_character (SFT *sft, char *string_buf, SFT_UChar chr, uint32_t height, uint32_t width, int32_t *x)
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

    *x        += mtx.leftSideBearing;

    char *buf  = &string_buf[(FONT_SCALE + mtx.yOffset) * width];
    // this placement does not account for overflowing the visual bounds of the input buffer
    for (int i = 0; i < img.height; ++i)
        memcpy (&buf[(i * width) + *x], &pixels[i * img.width], img.width);

    *x += mtx.advanceWidth;
}

static void
draw_str (buffer_t *context, char *str, int height, int width, int x, int y, int cur)
{
    int32_t cur_x, cur_y;
    int32_t t_x           = 0;
    int32_t t_y           = 0;

    char *string_buf      = calloc (1, width * height);
    char *string_buf_temp = string_buf;

    for (int i = 0; i < strlen (str) && t_x <= width; i++)
        {
            draw_character (context->render_context->sft, string_buf, str[i], height, width, &t_x);
            if (i == (int)cur - 1)
                cur_y = t_x, cur_y = t_y;
        }

    // convert the char* from sft to uint32_t*
    buffer_t render_buf       = INIT_BUF (context, width, height, x, y, render_buf);
    uint32_t *render_buf_temp = render_buf.buffer;
    // TODO: overrun.
    for (int i = 0; i < width * height; ++i)
        *render_buf_temp++ = *string_buf_temp++ << 24 | (COLOR_FG & 0xFFFFFF); // last bit is take out the alpha

    if (cur)
        draw_cur (&render_buf, t_x, t_y + (FONT_SCALE / 4));

    free (string_buf);
    draw_to_buffer (context, &render_buf, true);
    free (render_buf.buffer);
}

static void
draw_command_str (buffer_t *context)
{
    char *str = g_woof->state->current_command_string;
    int cur   = g_woof->state->cursor;

    int x     = PADDING / 2;
    // center vertical padding
    int y = (context->height / 2) - (3 * FONT_SCALE / 4);
    draw_str (context, str, context->height, context->width, x, y, cur);
}

static void
draw_input (buffer_t *context)
{
    buffer_t temp_buf = INIT_BUF (context, WIDTH, COMMAND_HEIGHT, 0, HEIGHT - COMMAND_HEIGHT, temp_buf);
    draw_color_square (&temp_buf, COMMAND_BG, temp_buf.width, temp_buf.height, 0, 0);
    // TODO: command draw

    draw_command_str (&temp_buf);

    draw_to_buffer (context, &temp_buf, false);
    free (temp_buf.buffer);
}

static void
draw_results (buffer_t *context)
{
    buffer_t temp_buf = INIT_BUF (context, WIDTH, (HEIGHT - PADDING - COMMAND_HEIGHT), 0, 0, temp_buf);
    draw_color_square (&temp_buf, COLOR_BG, temp_buf.width, temp_buf.height, 0, 0);

    draw_borders (&temp_buf);

    int x, y;

    x                  = PADDING;
    y                  = PADDING;

    result_t **results = g_woof->state->results;
    for (int i = 0; i < g_woof->state->result_count && y < temp_buf.height - PADDING; i++, results++)
        {
            draw_str (&temp_buf, (*results)->path, COMMAND_HEIGHT, WIDTH, PADDING, y, 0);
            y += COMMAND_HEIGHT + PADDING;
        }

    draw_to_buffer (context, &temp_buf, false);
    free (temp_buf.buffer);
}

void
render (render_context_t *context)
{
    INFO ("rendering into buffer :3");
    // double buf swap
    // draw_main_surface (context);
    draw_input (context->double_buf);
    draw_results (context->double_buf);

    buffer_t *temp_buf   = context->double_buf;
    context->double_buf  = context->surface_buf;
    context->surface_buf = temp_buf;
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
    g_woof->state->update = false;
    INFO ("rendered into buffer ");
}

static void
get_font (SFT *sft)
{
    sft->xScale = FONT_SCALE;
    sft->yScale = FONT_SCALE;
    sft->flags  = SFT_DOWNWARD_Y;

    // get font file dir
    FcConfig *config = FcInitLoadConfigAndFonts ();
    // not working rn ??
    FcPattern *pat = FcNameParse ((const FcChar8 *)(FONT_NAME));
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
    buffer_t *double_buf        = calloc (1, sizeof (buffer_t));
    context->color_depth        = COLOR_DEPTH;
    context->surface_buf        = surface_buf;
    surface_buf->height         = HEIGHT;
    surface_buf->width          = WIDTH;
    surface_buf->x              = 0;
    surface_buf->y              = 0;
    surface_buf->render_context = context;

    memcpy (double_buf, surface_buf, sizeof (buffer_t));

    context->double_buf = double_buf;

    // font shit
    SFT *sft = calloc (1, sizeof (SFT));
    get_font (sft);
    context->sft = sft;

    return context;
}
