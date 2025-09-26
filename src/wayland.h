
#ifndef WAYLAND_H
#define WAYLAND_H
#include "../include/xdg-shell.h"
#include <stdbool.h>
#include <stdint.h>
typedef struct wlc_t wlc_t;

// hold data for wayland shit
struct wlc_t {
  struct wl_display *display;
  struct wl_surface *surface;
  struct wl_registry *registry;
  struct wl_shm *shm;
  struct wl_shm_pool *shm_pool;
  struct wl_compositor *compositor;
  struct wl_buffer *buffer;
  struct xdg_wm_base *xdg_wm_base;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  int x;
  int y;
  int stride; // how many bytes is there in a line
  bool configured;
};

void wlc_init(wlc_t *wlc);

void wlc_disconnect(wlc_t *wlc);
#endif
