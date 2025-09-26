#include "../include/xdg-shell.h"
#include "util.h"
#include "wayland.h"
#include <wayland-client.h>
#define COLORDEPTHSLUDGE 4
void xdg_toplevel_configure_handler(void *userdata,
                                    struct xdg_toplevel *xdg_toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states) {
  IN_MESSAGE("xdg_toplevel_config: %dx%d", width, height);
};

void xdg_toplevel_close_handler(void *userdata,
                                struct xdg_toplevel *xdg_toplevel) {
  IN_MESSAGE("xdg_toplevel_close :3");
};

void xdg_surface_configure_handler(void *userdata,
                                   struct xdg_surface *xdg_surface,
                                   uint32_t serial) {
  xdg_surface_ack_configure(xdg_surface, serial);
  wlc_t *wlc = (struct wlc_t *)userdata;

  if (wlc->configured) {
    wl_surface_commit(wlc->surface);
  }

  wlc->configured = true;
};

void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
};

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_handle_ping,
};

// handle global messages
void wl_registry_global_handler(void *userdata, struct wl_registry *registry,
                                uint32_t name, const char *interface,
                                uint32_t version) {

  IN_MESSAGE("interface: %s, version: %u, name: %u", interface, version, name);
  // cast mysterious userdata to what it needs to be :3
  wlc_t *wlc = (struct wlc_t *)userdata;

  if (strcmp(interface, "wl_compositor") == 0) {
    INFO("%s connected", interface);
    wlc->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    INFO("%s connected", interface);
    wlc->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    INFO("%s connected", interface);
    wlc->xdg_wm_base =
        wl_registry_bind(wlc->registry, name, &xdg_wm_base_interface, version);
    xdg_wm_base_add_listener(wlc->xdg_wm_base, &xdg_wm_base_listener, wlc);
  }
}
void wl_registry_global_remove_handler(void *data, struct wl_registry *registry,
                                       uint32_t name) {
  IN_MESSAGE("remove name: %u", name);
}

void resize_handler(wlc_t *wlc, uint32_t x, uint32_t y) {
  wlc->x = x;
  wlc->y = y;
  wlc->stride = wlc->x * COLORDEPTHSLUDGE; // TODO: change from hardcoded sludge
}
