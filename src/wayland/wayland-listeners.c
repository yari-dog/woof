#include "wayland-listeners.h"
#include "../../include/wlr-layer-shell.h"
#include "../state.h"
#include "../util.h"
#include "../xkb.h"
#include "wayland.h"
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#define COLORDEPTHSLUDGE 4

// handle global messages
void
wl_registry_global_handler (void *userdata, struct wl_registry *registry, uint32_t name, const char *interface,
                            uint32_t version)
{

    IN_MESSAGE ("interface: %s, version: %u, name: %u", interface, version, name);
    // cast mysterious userdata to what it needs to be :3
    wlc_t *wlc = (struct wlc_t *)userdata;

    // true until an else in the below statements,,
    bool connected = true;

    if (strcmp (interface, "wl_compositor") == 0) // TODO: return to version
        wlc->compositor = wl_registry_bind (registry, name, &wl_compositor_interface, version);
    else if (strcmp (interface, "wl_shm") == 0)
        wlc->shm = wl_registry_bind (registry, name, &wl_shm_interface, version);
    else if (strcmp (interface, "zwlr_layer_shell_v1") == 0)
        wlc->zwlr_layer_shell = wl_registry_bind (registry, name, &zwlr_layer_shell_v1_interface, version);
    else if (strcmp (interface, "wl_seat") == 0)
        {
            wlc->seat = wl_registry_bind (registry, name, &wl_seat_interface, version);
            wl_seat_add_listener (wlc->seat, &wl_seat_listener, wlc);
        }
    else
        connected = false;

    if (connected)
        INFO ("%s connected", interface);
}

void
wl_registry_global_remove_handler (void *data, struct wl_registry *registry, uint32_t name)
{
    IN_MESSAGE ("remove name: %u", name);
}

void
zwlr_layer_surface_config_handler (void *userdata, struct zwlr_layer_surface_v1 *surface, uint32_t serial,
                                   uint32_t width, uint32_t height)
{
    IN_MESSAGE ("zwlr_layer_surface_configure: s(%i) %ix%i", serial, width, height);
    wlc_t *wlc = (struct wlc_t *)userdata;

    if (!wlc->configured || (width != wlc->state->width || height != wlc->state->height))
        {
            wlc_set_size (wlc, width, height);
        }

    zwlr_layer_surface_v1_ack_configure (surface, serial);

    wlc_resize_handler (wlc, width, height);
    wlc->configured = true;
}

void
zwlr_layer_surface_close_handler (void *userdata, struct zwlr_layer_surface_v1 *surface)
{
    IN_MESSAGE ("zwlr_layer_surface_close :3");
    wlc_t *wlc        = (struct wlc_t *)userdata;
    wlc->state->close = true;
}

void
wl_seat_capabilities_handler (void *userdata, struct wl_seat *seat, uint32_t capabilities)
{
    IN_MESSAGE ("seat capabilities");
    wlc_t *wlc = (struct wlc_t *)userdata;

    // all these are just bitmasking if this has announced keyboard capabilities
    bool has_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

    if (has_keyboard && wlc->keyboard == NULL)
        {
            wlc->keyboard = wl_seat_get_keyboard (wlc->seat);
            wl_keyboard_add_listener (wlc->keyboard, &wl_keyboard_listener, wlc);
        }

    // mouse
    bool has_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

    if (has_pointer && wlc->pointer == NULL)
        {
            wlc->pointer = wl_seat_get_pointer (wlc->seat);
            // wl_pointer_add_listener (wlc->pointer, &wl_pointer_listener, wlc);
        }
}

void
wl_keyboard_keymap_handler (void *userdata, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    wlc_t *wlc = (struct wlc_t *)userdata;
    if (format != XKB_KEYMAP_FORMAT_TEXT_V1)
        die ("wrong keymap format :o");

    // mmap fd ?
    char *format_map = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (format_map == MAP_FAILED)
        die ("xkb format map failed :(");

    xkb_set_keymap (wlc->state->xkb, format_map);

    munmap (format_map, size);
    close (fd);
}

void
wl_keyboard_enter_handler (void *userdata, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface,
                           struct wl_array *keys)
{
    IN_MESSAGE ("keyboard entered");
}
void
wl_keyboard_leave_handler (void *userdata, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface)
{
    IN_MESSAGE ("keyboard left");
}
void
wl_keyboard_key_handler (void *userdata, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state)
{
    IN_MESSAGE ("key %u", key);
    wlc_t *wlc = (struct wlc_t *)userdata;
    if (key == 16) // q for quit
        wlc->state->close = true;
}
void
wl_keyboard_modifiers_handler (void *userdata, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed,
                               uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    IN_MESSAGE ("mods locked");
}
void
wl_keyboard_repeat_info_handler (void *userdata, struct wl_keyboard *keyboard, int32_t rate, int32_t delay)
{
    IN_MESSAGE ("repeat_info");
}
