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
    uint32_t *buffer_data;
    uint32_t width;
    uint32_t height;
    uint32_t stride; // how many bytes is there in a line
    bool configured;
    char *title;
    bool close;
} wlc_t;

wlc_t *wlc_init ();
void wlc_start (wlc_t *wlc);

void wlc_resize_handler (wlc_t *wlc, uint32_t width, uint32_t height);
void wlc_set_size (wlc_t *wlc, uint32_t width, uint32_t height);

void wlc_draw_frame (wlc_t *wlc);

// these are stored in the woof struct on wayland init
void wlc_disconnect (state_t *state);
void wlc_main_loop (state_t *state);
#endif
