#include "wayland.h"
#include "../include/xdg-shell.h"
#include "util.h"
#include "wayland-handlers.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <syscall.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#define COLORDEPTHSLUDGE 4

static const struct wl_registry_listener registry_listener;
static const struct xdg_surface_listener xdg_surface_listener;
static const struct xdg_toplevel_listener xdg_toplevel_listener;

void draw_frame(wlc_t *wlc) {
  INFO("drawing frame :3");
  // print checkerboard
  uint32_t bg = 0xFF282828;
  uint32_t color = 0xFFEBDBB2;
  for (int i = 0; i < wlc->height; ++i) {
    for (int j = 0; j < wlc->width; ++j) {
      if ((i + j / 8 * 8) % 16 < 8)
        wlc->buffer_data[i * wlc->width + j] = color;
      else
        wlc->buffer_data[i * wlc->width + j] = bg;
    }
  }
}

void init_buffer(wlc_t *wlc) {
  INFO("initing buffer %u %u", wlc->width, wlc->height);
  int size = wlc->height * wlc->stride;

  // make the file
  int fd = create_shm_file(size);

  // mamory map da file
  wlc->buffer_data =
      mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (wlc->buffer_data == MAP_FAILED)
    EXIT("fucked up mapping the shmm :\\ sowwy");
  INFO("mmaped");

  wlc->shm_pool =
      wl_shm_create_pool(wlc->shm, fd,
                         size); // TODO: is this a race condition waiting to
                                // happen with the above line
  INFO("pooled");
  wlc->buffer =
      wl_shm_pool_create_buffer(wlc->shm_pool, 0, wlc->width, wlc->height,
                                wlc->stride, WL_SHM_FORMAT_XRGB8888);
  INFO("buffered");
  wl_shm_pool_destroy(wlc->shm_pool);
  INFO("destroyed");
  close(fd);
  INFO("closed");

  INFO("%s buffed size: %ux%u", wlc->title, wlc->width, wlc->height);
}

void set_size(wlc_t *wlc, uint32_t width, uint32_t height) {
  wlc->width = width;
  wlc->height = height;
  wlc->stride =
      wlc->width * COLORDEPTHSLUDGE; // TODO: change from hardcoded sludge

  INFO("%s resized size: %ux%u", wlc->title, wlc->width, wlc->height);
}

void resize_handler(wlc_t *wlc, uint32_t width, uint32_t height) {
  INFO("rezising :3");
  if (wlc->buffer_data) {
    INFO("unmapping buffer");
    munmap(wlc->buffer_data, wlc->height * wlc->stride);
  }
  set_size(wlc, width, height);
  init_buffer(wlc);
  draw_frame(wlc);
}

void xdg_toplevel_configure_handler(void *userdata,
                                    struct xdg_toplevel *xdg_toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states) {
  IN_MESSAGE("xdg_toplevel_config: %dx%d", width, height);
  wlc_t *wlc = (struct wlc_t *)userdata;
  INFO("%s current size: %ux%u", wlc->title, wlc->width, wlc->height);
  if (wlc->width != width || wlc->height != height) {
    resize_handler(wlc, width, height);
  }
};

void xdg_toplevel_close_handler(void *userdata,
                                struct xdg_toplevel *xdg_toplevel) {
  IN_MESSAGE("xdg_toplevel_close :3");
  wlc_t *wlc = (struct wlc_t *)userdata;
  wlc->close = true;
};

void xdg_toplevel_capabilities_handler(void *userdata,
                                       struct xdg_toplevel *xdg_toplevel,
                                       struct wl_array *capabilities) {
  IN_MESSAGE("xdg_toplevel_capabilities :3");
}

void xdg_surface_configure_handler(void *userdata,
                                   struct xdg_surface *xdg_surface,
                                   uint32_t serial) {
  xdg_surface_ack_configure(xdg_surface, serial);
  wlc_t *wlc = (struct wlc_t *)userdata;
  IN_MESSAGE("%s configured :3, %i, %u", wlc->title, wlc->configured, serial);

  if (!wlc->configured)
    resize_handler(wlc, wlc->width, wlc->height);

  wl_surface_attach(wlc->surface, wlc->buffer, wlc->width, wlc->height);
  wl_surface_commit(wlc->surface);
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
static const struct wl_registry_listener registry_listener = {
    .global = wl_registry_global_handler,
    .global_remove = wl_registry_global_remove_handler,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
    .wm_capabilities = xdg_toplevel_capabilities_handler,
};

// just speedrun setting up listeners idgaf

// attach above listeners to the registry
void registry_init(wlc_t *wlc) {

  wlc->registry = wl_display_get_registry(wlc->display);
  wl_registry_add_listener(wlc->registry, &registry_listener, wlc);

  // block us till queue dropped
  wl_display_roundtrip(wlc->display);
}

void make_surfaces(wlc_t *wlc) {
  INFO("making surfaces");
  wlc->surface = wl_compositor_create_surface(wlc->compositor);
  wlc->xdg_surface =
      xdg_wm_base_get_xdg_surface(wlc->xdg_wm_base, wlc->surface);
  xdg_surface_add_listener(wlc->xdg_surface, &xdg_surface_listener, wlc);

  wlc->xdg_toplevel = xdg_surface_get_toplevel(wlc->xdg_surface);
  xdg_toplevel_set_title(wlc->xdg_toplevel, wlc->title);
  xdg_toplevel_add_listener(wlc->xdg_toplevel, &xdg_toplevel_listener, wlc);
}

// just abstraction idk cleaner in my brain
void wlc_disconnect(wlc_t *wlc) {
  INFO("closing :O");
  wl_display_disconnect(wlc->display);
}

void set_title(wlc_t *wlc, char *title) { wlc->title = title; }

wlc_t *wlc_init() {
  // assign memory and initialise it to 0 for swag purposes
  wlc_t *wlc = malloc(sizeof(wlc_t));
  memset(wlc, 0, sizeof(wlc_t));

  // set values etc
  set_title(wlc, ":woof");
  set_size(wlc, 100, 100);
  return wlc;
}

void wlc_start(wlc_t *wlc) {
  INFO("wlc_init");
  // connect display
  wlc->display = wl_display_connect(NULL);

  if (!wlc->display)
    exit(errno);

  // grab registry and give handlers to it
  registry_init(wlc);

  // die if no <thing>
  if (!wlc->compositor || !wlc->shm || !wlc->xdg_wm_base)
    EXIT("MISSING wl_compositor (%i) || wl_shm (%i) || xdg_wm_base (%i)",
         !!wlc->compositor, !!wlc->shm, !!wlc->xdg_wm_base);

  // set up the surface
  make_surfaces(wlc);
  INFO("surface commit");
  wl_surface_commit(wlc->surface);
  while (wl_display_dispatch(wlc->display) != -1 && !wlc->configured) {
    // blank on purpose
  }
  // once that loop exits, the configure listener above for surface is called
  // this is where the size gets set and the window begins to exist
}
