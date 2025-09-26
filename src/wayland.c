#include "wayland.h"
#include "../include/xdg-shell.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <syscall.h>
#include <unistd.h>
#include <wayland-client.h>

#define COLORDEPTHSLUDGE 4

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
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    INFO("%s connected", interface);
    wlc->xdg_wm_base =
        wl_registry_bind(wlc->registry, name, &xdg_wm_base_interface, version);
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

void resize_handler(wlc_t *wlc, uint32_t x, uint32_t y) {
  wlc->x = x;
  wlc->y = y;
  wlc->stride = wlc->x * COLORDEPTHSLUDGE; // TODO: change from hardcoded sludge
}

// i cba to code this tbh
static void randname(char *buf) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r & 15) + (r & 16) * 2;
    r >>= 5;
  }
}

// create and open the shared memory
int open_shm_file() {
  char name[] = "/woof-wayland-xxxxxx";
  for (int i = 100; i >= 0; i--) {
    randname(name + strlen(name) - 6);
    INFO("shm name:%s", name);
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  }
  return -1;
}

int create_shm_file(int size) {
  INFO("making shm file");
  int fd = open_shm_file();
  if (fd < 0)
    return fd;

  if (ftruncate(fd, size) < 0) {
    close(fd);
    EXIT("fucked up creating the buffer :\\ sowwy");
  }

  INFO("shm made :3");
  return fd;
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

void init_buffer(wlc_t *wlc) {
  INFO("wlc_init_buffer");
  wlc->surface = wl_compositor_create_surface(wlc->compositor);
  wlc->xdg_surface =
      xdg_wm_base_get_xdg_surface(wlc->xdg_wm_base, wlc->surface);
  wlc->xdg_toplevel = xdg_surface_get_toplevel(wlc->xdg_surface);

  wl_surface_commit(wlc->surface);

  build_buffer(wlc);

  wl_surface_attach(wlc->surface, wlc->buffer, wlc->x, wlc->y);
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
