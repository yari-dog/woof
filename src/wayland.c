#include "wayland.h"
#include "util.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>

// handle global messages
void registry_global_handler(void *userdata, struct wl_registry *registry,
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
  }
}
void registry_global_remove_handler(void *data, struct wl_registry *registry,
                                    uint32_t name) {
  IN_MESSAGE("remove name: %u", name);
}

const struct wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler,
};

// attach above listeners to the registry
void registry_init(wlc_t *wlc) {

  wlc->registry = wl_display_get_registry(wlc->display);
  wl_registry_add_listener(wlc->registry, &registry_listener, wlc);

  // block us till queue dropped
  wl_display_roundtrip(wlc->display);
}

void wlc_init(wlc_t *wlc) {
  INFO("wlc_init");
  // connect display
  wlc->display = wl_display_connect(NULL);

  if (!wlc->display)
    exit(errno);

  // grab registry and give handlers to it
  registry_init(wlc);

  if (!wlc->compositor || !wlc->shm)
    EXIT("MISSING wl_compositor (%i) || wl_shm (%i)", !!wlc->compositor,
         !!wlc->shm);

  INFO("swag: achieved");
}

// just abstraction idk cleaner in my brain
void wlc_disconnect(wlc_t *wlc) { wl_display_disconnect(wlc->display); }
