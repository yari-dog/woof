#ifndef WAYLAND_HANDLERS_H
#define WAYLAND_HANDLERS_H

#include "wayland.h"
#include <stdint.h>
void xdg_toplevel_configure_handler(void *userdata,
                                    struct xdg_toplevel *xdg_toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states);

void xdg_toplevel_close_handler(void *userdata,
                                struct xdg_toplevel *xdg_toplevel);

void xdg_surface_configure_handler(void *userdata,
                                   struct xdg_surface *xdg_surface,
                                   uint32_t serial);
void xdg_toplevel_capabilities_handler(void *userdata,
                                       struct xdg_toplevel *xdg_toplevel,
                                       struct wl_array *capabilities);
void wl_registry_global_handler(void *userdata, struct wl_registry *registry,
                                uint32_t name, const char *interface,
                                uint32_t version);
void wl_registry_global_remove_handler(void *data, struct wl_registry *registry,
                                       uint32_t name);
void resize_handler(wlc_t *wlc, uint32_t x, uint32_t y);
#endif
