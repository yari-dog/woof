#ifndef WAYLAND_LISTENERS_H
#define WAYLAND_LISTENERS_H
#include "../../include/xdg-shell.h"
#include <wayland-client.h>

void wl_registry_global_handler(void *userdata, struct wl_registry *registry, uint32_t name, const char *interface,
                                uint32_t version);
void wl_registry_global_remove_handler(void *data, struct wl_registry *registry, uint32_t name);
static struct wl_registry_listener registry_listener = {
    .global = wl_registry_global_handler,
    .global_remove = wl_registry_global_remove_handler,
};

void xdg_surface_configure_handler(void *userdata, struct xdg_surface *xdg_surface, uint32_t serial);
static struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

void xdg_toplevel_configure_handler(void *userdata, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height,
                                    struct wl_array *states);
void xdg_toplevel_close_handler(void *userdata, struct xdg_toplevel *xdg_toplevel);
void xdg_toplevel_capabilities_handler(void *userdata, struct xdg_toplevel *xdg_toplevel,
                                       struct wl_array *capabilities);

static struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
    .wm_capabilities = xdg_toplevel_capabilities_handler,
};

#endif
