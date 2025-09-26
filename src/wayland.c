#include "wayland.h"
#include "../include/xdg-shell.h"
#include "util.h"
#include "wayland-handlers.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <syscall.h>
#include <unistd.h>
#include <wayland-client-core.h>
#include <wayland-client.h>

static struct wl_registry_listener registry_listener = {
    .global = wl_registry_global_handler,
    .global_remove = wl_registry_global_remove_handler,
};

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler,
};

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
};

// attach above listeners to the registry
void registry_init(wlc_t *wlc) {

  wlc->registry = wl_display_get_registry(wlc->display);
  wl_registry_add_listener(wlc->registry, &registry_listener, wlc);

  // block us till queue dropped
  wl_display_roundtrip(wlc->display);
}

void build_buffer(wlc_t *wlc) {
  int size = wlc->y * wlc->stride;

  // make the file
  int fd = create_shm_file(size);

  // mamory map da file
  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED)
    EXIT("fucked up mapping the shmm :\\ sowwy");
  INFO("mmaped");

  wlc->shm_pool =
      wl_shm_create_pool(wlc->shm, fd,
                         size); // TODO: is this a race condition waiting to
                                // happen with the above line
  INFO("pooled");
  wlc->buffer = wl_shm_pool_create_buffer(wlc->shm_pool, 0, wlc->x, wlc->y,
                                          wlc->stride, WL_SHM_FORMAT_XRGB8888);
  INFO("buffered");
  wl_shm_pool_destroy(wlc->shm_pool);
  INFO("destroyed");
  close(fd);
  INFO("closed");
}

void make_surfaces(wlc_t *wlc) {
  INFO("making surfaces");
  wlc->surface = wl_compositor_create_surface(wlc->compositor);
  wlc->xdg_surface =
      xdg_wm_base_get_xdg_surface(wlc->xdg_wm_base, wlc->surface);
  wlc->xdg_toplevel = xdg_surface_get_toplevel(wlc->xdg_surface);

  INFO("toplevel listener");
  xdg_toplevel_add_listener(wlc->xdg_toplevel, &xdg_toplevel_listener, wlc);
  INFO("surface listener");
  xdg_surface_add_listener(wlc->xdg_surface, &xdg_surface_listener, wlc);
}

void init_buffer(wlc_t *wlc) {
  INFO("wlc_init_buffer");

  make_surfaces(wlc);
  INFO("surface commit");
  wl_surface_commit(wlc->surface);
  while (wl_display_dispatch(wlc->display) != -1 && !wlc->configured) {
    // blank on purpose
  }

  build_buffer(wlc);

  wl_surface_attach(wlc->surface, wlc->buffer, wlc->x, wlc->y);
  wl_surface_commit(wlc->surface);
  INFO("attached");
}

// just abstraction idk cleaner in my brain
void wlc_disconnect(wlc_t *wlc) { wl_display_disconnect(wlc->display); }

void wlc_init(wlc_t *wlc) {
  INFO("wlc_init");

  INFO("setting wlc params");
  resize_handler(wlc, 100, 100);

  // connect display
  wlc->display = wl_display_connect(NULL);

  if (!wlc->display)
    exit(errno);

  // grab registry and give handlers to it
  registry_init(wlc);

  if (!wlc->compositor || !wlc->shm || !wlc->xdg_wm_base)
    EXIT("MISSING wl_compositor (%i) || wl_shm (%i) || xdg_wm_base (%i)",
         !!wlc->compositor, !!wlc->shm, !!wlc->xdg_wm_base);

  init_buffer(wlc);
}
