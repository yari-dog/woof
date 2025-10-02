#ifndef WAYLAND_H
#define WAYLAND_H
#include "../../include/wlr-layer-shell.h"
#include <stdbool.h>
#include <stdint.h>

// cant include state.h here or it's recursive. wish there was a way to avoid this.
typedef struct state_t state_t;

// hold data for wayland shit
typedef struct wlc_t
{
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_registry *registry;
    struct wl_shm *shm;
    struct wl_shm_pool *shm_pool;
    struct wl_compositor *compositor;
    struct wl_buffer *buffer;
    struct zwlr_layer_shell_v1 *zwlr_layer_shell;
    struct zwlr_layer_surface_v1 *zwlr_layer_surface;
    struct wl_output *output;
    struct state_t *state;
    uint32_t *buffer_data;
    bool configured;
} wlc_t;

/*
 * initiates the wlc struct and 0's it out
 */
wlc_t *wlc_init ();

/*
 * deal with a resize (remake buffer and re-render, basically)
 */
void wlc_resize_handler (wlc_t *wlc, uint32_t width, uint32_t height);

/*
 * set the size on resize
 */
void wlc_set_size (wlc_t *wlc, uint32_t width, uint32_t height);

// these are stored in the woof struct on wayland init
void wlc_start (state_t *state);
void wlc_disconnect (state_t *state);
void wlc_main_loop (state_t *state);
#endif
