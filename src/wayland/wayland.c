#include "wayland.h"
#include "../../include/wlr-layer-shell.h"
#include "../render.h"
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
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#define COLORDEPTHSLUDGE 4
#define BUFFER_SCALE     4
#define MARGIN           15

void wlc_set_title (wlc_t *wlc, char *title, int size);

void
wlc_init_buffer (wlc_t *wlc)
{
    INFO ("initing buffer %u %u", wlc->width, wlc->height);
    int size = wlc->height * wlc->stride;

    // make the file
    int fd = create_shm_file (size);

    wlc->buffer_data = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (wlc->buffer_data == MAP_FAILED)
        die ("fucked up mapping the shmm :\\ sowwy");

    wlc->shm_pool = wl_shm_create_pool (wlc->shm, fd,
                                        size); // TODO: is this a race condition waiting to
                                               // happen with the above line
    wlc->buffer
        = wl_shm_pool_create_buffer (wlc->shm_pool, 0, wlc->width, wlc->height, wlc->stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy (wlc->shm_pool);
    close (fd);

    INFO ("%s buffed size: %ux%u", wlc->title, wlc->width, wlc->height);
}

void
wlc_wipe_buffer (wlc_t *wlc)
{
    if (wlc->buffer_data)
        {
            INFO ("unmapping buffer");
            munmap (wlc->buffer_data, wlc->height * wlc->stride);
        }
}

void
wlc_set_surface (wlc_t *wlc)
{
    wlc_wipe_buffer (wlc);
    wlc_init_buffer (wlc);

    render (wlc->buffer_data, wlc->height * wlc->stride, wlc->width, wlc->height);

    wl_surface_set_buffer_scale (wlc->surface, 1);
    wl_surface_attach (wlc->surface, wlc->buffer, 0, 0);
    wl_surface_damage_buffer (wlc->surface, 0, 0, wlc->width, wlc->height);
    wl_surface_commit (wlc->surface);
    INFO ("frame rendered :o");
}

void
wlc_set_size (wlc_t *wlc, uint32_t width, uint32_t height)
{
    wlc->width  = width;
    wlc->height = height;
    wlc->stride = wlc->width * COLORDEPTHSLUDGE; // TODO: change from hardcoded sludge
                                                 // if (wlc->zwlr_layer_surface)
    zwlr_layer_surface_v1_set_size (wlc->zwlr_layer_surface, wlc->width, wlc->height);
    INFO ("%s resized size: %ux%u", wlc->title, wlc->width, wlc->height);
}

void
wlc_resize_handler (wlc_t *wlc, uint32_t width, uint32_t height)
{
    INFO ("resizing :3");
    wlc_set_surface (wlc);
}

void
wlc_make_surfaces (wlc_t *wlc)
{
    INFO ("making surfaces");
    wlc->surface = wl_compositor_create_surface (wlc->compositor);
    wl_surface_add_listener (wlc->surface, &wl_surface_listener, wlc);

    // wlr-layer-shell-unstable-v1
    wlc->zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface (wlc->zwlr_layer_shell, wlc->surface, wlc->output,
                                                                     ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, wlc->title);
    wlc_set_size (wlc, 80, 80);
    zwlr_layer_surface_v1_set_anchor (wlc->zwlr_layer_surface,
                                      ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone (wlc->zwlr_layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity (wlc->zwlr_layer_surface,
                                                      0); // TODO: change this when i no longer need to CTRL+C to close
    zwlr_layer_surface_v1_set_margin (wlc->zwlr_layer_surface, MARGIN, MARGIN, MARGIN, MARGIN);
    zwlr_layer_surface_v1_add_listener (wlc->zwlr_layer_surface, &zwlr_layer_surface_listener, wlc);

    INFO ("surface commit");
    wl_surface_commit (wlc->surface);
}

void
wlc_set_title (wlc_t *wlc, char *title, int size)
{
    if (wlc->title)
        free (wlc->title);
    char *title_buf = malloc (size);
    strcpy (title_buf, title);
    wlc->title = title_buf;
    INFO ("title set: %s", wlc->title);
}

wlc_t *
wlc_init ()
{
    // assign memory and initialise it to 0 for swag purposes
    wlc_t *wlc = malloc (sizeof (wlc_t));
    memset (wlc, 0, sizeof (wlc_t));

    // set values etc
    char *default_title = ":woof";
    wlc_set_title (wlc, default_title, sizeof (default_title + 1));
    wlc->output = NULL;

    return wlc;
}

void
wlc_start (state_t *state)
{
    wlc_t *wlc = state->wlc;
    INFO ("wlc_init");
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
    INFO ("closing :O");
    wl_display_disconnect (wlc->display);
}

// the "main loop", IE what gets ran to keep wayland alive
void
wlc_main_loop (state_t *state)
{
    wlc_t *wlc = state->wlc;
    wl_display_flush (wlc->display);

    if (!wl_display_dispatch_pending (wlc->display))
        if (wl_display_dispatch (wlc->display) < 0)
            die ("display dispatch failed :(");
}
