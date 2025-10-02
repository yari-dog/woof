#include "../../include/wlr-layer-shell.h"
#include "../util.h"
#include "wayland.h"
#include <wayland-client-protocol.h>
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

    if (!wlc->configured || (width != wlc->width || height != wlc->height))
        {
            set_size (wlc, width, height);
        }

    zwlr_layer_surface_v1_ack_configure (surface, serial);

    resize_handler (wlc, width, height);
    wlc->configured = true;
}

void
zwlr_layer_surface_close_handler (void *userdata, struct zwlr_layer_surface_v1 *surface)
{
    IN_MESSAGE ("zwlr_layer_surface_close :3");
    wlc_t *wlc = (struct wlc_t *)userdata;
    wlc->close = true;
}
