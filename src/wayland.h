
#ifndef WAYLAND_H
#define WAYLAND_H
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
};

void wlc_init(wlc_t *wlc);

void wlc_disconnect(wlc_t *wlc);
#endif
