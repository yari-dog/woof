
// i cba to code this tbh
#include "util.h"
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

// gen random name for the shm file, incase collision somehow happens
static void
randname (char *buf)
{
    struct timespec ts;
    clock_gettime (CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;
    for (int i = 0; i < 6; ++i)
        {
            buf[i]   = 'A' + (r & 15) + (r & 16) * 2;
            r      >>= 5;
        }
}

// create and open the shared memory
int
open_shm_file ()
{
    char name[] = "/woof-wayland-xxxxxx";
    for (int i = 100; i >= 0; i--)
        {
            randname (name + strlen (name) - 6);
            INFO ("shm name:%s", name);
            int fd = shm_open (name, O_RDWR | O_CREAT | O_EXCL, 0600);
            if (fd >= 0)
                {
                    shm_unlink (name);
                    return fd;
                }
        }
    return -1;
}

// wrapper for open_shm_file stuff
int
create_shm_file (int size)
{
    INFO ("making shm file");
    int fd = open_shm_file ();
    if (fd < 0)
        return fd;

    if ((ftruncate (fd, size)) < 0)
        {
            close (fd);
            die ("fucked up creating the buffer :\\ sowwy");
        }

    INFO ("shm made :3");
    return fd;
}
