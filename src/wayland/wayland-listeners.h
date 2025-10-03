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

void wl_seat_capabilities_handler (void *userdata, struct wl_seat *seat, uint32_t capabilities);
static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities_handler,
    .name = noop
};

void wl_keyboard_keymap_handler(void *userdata, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size);
void wl_keyboard_enter_handler(void *userdata, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
void wl_keyboard_leave_handler(void *userdata, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);
void wl_keyboard_key_handler(void *userdata, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void wl_keyboard_modifiers_handler(void *userdata, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void wl_keyboard_repeat_info_handler(void *userdata, struct wl_keyboard *keyboard, int32_t rate, int32_t delay);
static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap_handler,
    .enter = wl_keyboard_enter_handler,
    .leave = wl_keyboard_leave_handler,
    .key = wl_keyboard_key_handler,
    .modifiers = wl_keyboard_modifiers_handler,
    .repeat_info = wl_keyboard_repeat_info_handler,
};

void wl_pointer_handler();
#endif
