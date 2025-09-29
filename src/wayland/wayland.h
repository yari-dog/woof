
#ifndef WAYLAND_H
#define WAYLAND_H
#include "../../include/xdg-shell.h"
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
    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    int32_t x; // window x pos
    int32_t y; // window y pos
    uint32_t width;
    uint32_t height;
    uint32_t stride; // how many bytes is there in a line
    bool configured;
    bool resizing;
    bool close;
    char *title;
} wlc_t;

wlc_t *wlc_init();
void wlc_start(wlc_t *wlc);

void resize_handler(wlc_t *wlc, uint32_t width, uint32_t height);

void wlc_disconnect(wlc_t *wlc);
#endif
