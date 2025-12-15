#include "wayland.h"
#include "../../include/wlr-layer-shell.h"
#include "../config.h"
#include "../render.h"
#include "../state.h"
#include "../util.h"
#include "../woof.h"
#include "wayland-listeners.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>
#include <unistd.h>
#include <wayland-client.h>

static void wlc_set_title (wlc_t *wlc, char *title, int size);

static void
wlc_init_buffer (wlc_t *wlc)
{
    render_context_t *render_context = g_woof->state->render_context;
    buffer_t *surface_buf            = render_context->surface_buf;
    INFO ("initing buffer %u %u", surface_buf->width, surface_buf->height);

    int size = surface_buf->height * surface_buf->stride;

    // make the file
    int fd = create_shm_file (size);

    INFO ("size %i", size);
    surface_buf->buffer = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (surface_buf->buffer == MAP_FAILED)
        die ("fucked up mapping the shmm :\\ sowwy");

    wlc->shm_pool = wl_shm_create_pool (wlc->shm, fd,
                                        size); // TODO: is this a race condition waiting to
                                               // happen with the above line
    wlc->buffer = wl_shm_pool_create_buffer (wlc->shm_pool, 0, surface_buf->width, surface_buf->height,
                                             surface_buf->stride, COLOR_FORMAT);
    wl_shm_pool_destroy (wlc->shm_pool);
    close (fd);

    INFO ("%s buffed size: %ux%u", g_woof->state->title, surface_buf->width, surface_buf->height);
}

static void
wlc_wipe_buffer (wlc_t *wlc)
{
    render_context_t *render_context = g_woof->state->render_context;
    buffer_t *surface_buf            = render_context->surface_buf;

    if (surface_buf->buffer)
        {
            INFO ("unmapping buffer");
            munmap (surface_buf->buffer, surface_buf->height * surface_buf->stride);
        }
}

static void
wlc_set_surface (wlc_t *wlc)
{
    int64_t start_time               = ms_since_epoch ();
    render_context_t *render_context = g_woof->state->render_context;
    render (render_context);

    buffer_t *surface_buf      = render_context->surface_buf;

    struct wl_buffer *temp_buf = wlc->buffer;
    wlc->buffer                = wlc->double_buf;
    wlc->double_buf            = temp_buf;

    // wl_surface_set_buffer_scale (wlc->surface, 1);
    wl_surface_attach (wlc->surface, wlc->buffer, 0, 0);
    wl_surface_damage_buffer (wlc->surface, 0, 0, surface_buf->width, surface_buf->height);
    wl_surface_commit (wlc->surface);
    int64_t current_time = ms_since_epoch ();
    INFO ("frame rendered in %li ms :o", current_time - start_time);
}

void
wlc_set_size (wlc_t *wlc, uint32_t width, uint32_t height)
{
    render_context_t *render_context = g_woof->state->render_context;
    buffer_t *surface_buf            = render_context->surface_buf;

    if (width)
        surface_buf->width = width;
    if (height)
        surface_buf->height = height;
    surface_buf->stride = surface_buf->width * render_context->color_depth; // TODO: change from hardcoded sludge
                                                                            // if (wlc->zwlr_layer_surface)
    zwlr_layer_surface_v1_set_size (wlc->zwlr_layer_surface, surface_buf->width, surface_buf->height);
    INFO ("%s resized size: %ux%u", g_woof->state->title, surface_buf->width, surface_buf->height);
}

void
wlc_resize_handler (wlc_t *wlc, uint32_t width, uint32_t height)
{
    INFO ("resizing :3");
    wlc_wipe_buffer (wlc);
    wlc_init_buffer (wlc);

    buffer_t *temp_buf;
    // build double buffer
    temp_buf                                   = g_woof->state->render_context->surface_buf;
    g_woof->state->render_context->surface_buf = g_woof->state->render_context->double_buf;
    g_woof->state->render_context->double_buf  = temp_buf;
    wlc->double_buf                            = wlc->buffer;

    wlc_set_size (wlc, temp_buf->width, temp_buf->height);

    wlc_wipe_buffer (wlc);
    wlc_init_buffer (wlc);
    wlc_set_surface (wlc);
}

static void
wlc_make_surfaces (wlc_t *wlc)
{
    INFO ("making surfaces");
    wlc->surface = wl_compositor_create_surface (wlc->compositor);
    wl_surface_add_listener (wlc->surface, &wl_surface_listener, wlc);

    // wlr-layer-shell-unstable-v1
    wlc->zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface (
        wlc->zwlr_layer_shell, wlc->surface, wlc->output, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, g_woof->state->title);
    wlc_set_size (wlc, 0, 0);
    zwlr_layer_surface_v1_set_anchor (wlc->zwlr_layer_surface,
                                      ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone (wlc->zwlr_layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity (wlc->zwlr_layer_surface,
                                                      1); // TODO: change this when i no longer need to CTRL+C to close
    zwlr_layer_surface_v1_set_margin (wlc->zwlr_layer_surface, MARGIN, MARGIN, MARGIN, MARGIN);
    zwlr_layer_surface_v1_add_listener (wlc->zwlr_layer_surface, &zwlr_layer_surface_listener, wlc);

    wl_surface_commit (wlc->surface);
    INFO ("surface commit");
}

static void
wlc_set_title (wlc_t *wlc, char *title, int size)
{
    if (g_woof->state->title)
        free (g_woof->state->title);

    char *title_buf = malloc (size);
    strcpy (title_buf, title);
    g_woof->state->title = title_buf;
    INFO ("title set: %s", g_woof->state->title);
}

wlc_t *
wlc_init ()
{
    // assign memory and initialise it to 0 for swag purposes
    wlc_t *wlc = calloc (1, sizeof (wlc_t));

    // set values etc
    wlc->output = NULL;

    return wlc;
}

void
wlc_start (state_t *state)
{
    wlc_t *wlc = state->wlc;
    INFO ("wlc_init");
    wlc_set_title (state->wlc, TITLE, sizeof (TITLE));
    // connect display
    wlc->display = wl_display_connect (NULL);

    if (!wlc->display)
        exit (errno);

    // grab registry and give handlers to it
    wlc->registry = wl_display_get_registry (wlc->display);
    wl_registry_add_listener (wlc->registry, &registry_listener, wlc);

    // block us till queue dropped
    wl_display_roundtrip (wlc->display);

    // die if no <thing>
    if (!wlc->compositor || !wlc->shm || !wlc->zwlr_layer_shell)
        die ("MISSING wl_compositor (%i) || wl_shm (%i) || zwlr_layer_shell (%i)", !!wlc->compositor, !!wlc->shm,
             !!wlc->zwlr_layer_shell);

    // set up the surface
    wlc_make_surfaces (wlc);
}

// just abstraction idk cleaner in my brain
void
wlc_disconnect (state_t *state)
{
    wlc_t *wlc = state->wlc;
    INFO ("wlc_disconnect closing :O");
    wl_display_disconnect (wlc->display);
}

// the "main loop", IE what gets ran to keep wayland alive
void
wlc_main_loop (state_t *state)
{
    if (state->update)
        wlc_set_surface (state->wlc);
    wl_display_flush (state->wlc->display);

    if (!wl_display_dispatch_pending (state->wlc->display))
        if (wl_display_dispatch (state->wlc->display) < 0)
            die ("display dispatch failed :(");
}
