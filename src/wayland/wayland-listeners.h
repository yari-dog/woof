#ifndef WAYLAND_LISTENERS_H
#define WAYLAND_LISTENERS_H
#include "../../include/wlr-layer-shell.h"
#include "../util.h"
#include <wayland-client.h>

void wl_registry_global_handler (void *userdata, struct wl_registry *registry, uint32_t name, const char *interface,
                                 uint32_t version);
void wl_registry_global_remove_handler (void *data, struct wl_registry *registry, uint32_t name);
static struct wl_registry_listener registry_listener = {
    .global        = wl_registry_global_handler,
    .global_remove = wl_registry_global_remove_handler,
};

void zwlr_layer_surface_config_handler (void *userdata, struct zwlr_layer_surface_v1 *surface, uint32_t serial,
                                        uint32_t width, uint32_t height);
void zwlr_layer_surface_close_handler (void *userdata, struct zwlr_layer_surface_v1 *surface);

static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_listener
    = { .configure = zwlr_layer_surface_config_handler, .closed = zwlr_layer_surface_close_handler };

static const struct wl_surface_listener wl_surface_listener
    = { .enter = noop, .leave = noop, .preferred_buffer_scale = noop, .preferred_buffer_transform = noop };
#endif
