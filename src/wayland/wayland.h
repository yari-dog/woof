#ifndef WAYLAND_H
#define WAYLAND_H
#include "../../include/wlr-layer-shell.h"
#include <stdbool.h>
#include <stdint.h>

// hold data for wayland shit
typedef struct wlc_t {
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_registry *registry;
    struct wl_shm *shm;
    struct wl_shm_pool *shm_pool;
    struct wl_compositor *compositor;
    struct wl_buffer *buffer;
    uint32_t *buffer_data;
    struct zwlr_layer_shell_v1 *zwlr_layer_shell;
    struct zwlr_layer_surface_v1 *zwlr_layer_surface;
    struct wl_output *output;
    uint32_t width;
    uint32_t height;
    uint32_t stride; // how many bytes is there in a line
    bool configured;
    bool close;
    char *title;
} wlc_t;

wlc_t *wlc_init();
void wlc_start(wlc_t *wlc);

void resize_handler(wlc_t *wlc, uint32_t width, uint32_t height);

void draw_frame(wlc_t *wlc);

void wlc_disconnect(wlc_t *wlc);
#endif
