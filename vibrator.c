// Taken from https://xnux.eu/devices/feature/vibrator.html
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>

void syscall_error(int is_err, const char* fmt, ...)
{
        va_list ap;

        if (!is_err)
                return;

        printf("ERROR: ");
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf(": %s\n", strerror(errno));

        exit(1);
}

int open_event_dev(const char* name_needle, int flags)
{
        char path[256];
        char name[256];
        int fd, ret;

        // find the right device and open it
        for (int i = 0; i < 10; i++) {
                snprintf(path, sizeof path, "/dev/input/event%d", i);
                fd = open(path, flags);
                if (fd < 0)
                        continue;

                ret = ioctl(fd, EVIOCGNAME(256), name);
                if (ret < 0)
                        continue;

                if (strstr(name, name_needle))
                        return fd;

                close(fd);
        }

        errno = ENOENT;
        return -1;
}

#define GET_INT(argv, i) atoi(argv[*i][2]?argv[*i]+2:argv[++*i])
int main(int argc, char* argv[])
{
    int length = 500;
    int delay = 500;
    int count = 3;
    int magnitude = 1;
    int type = FF_RUMBLE;
    for(int i = 1; argv[i]; i++) {
        switch(argv[i][1]) {
            case 'l':
                printf("%s %d\n",argv[i], atoi(argv[i]));
                length = GET_INT(argv, &i);
                break;
            case 'd':
                delay = GET_INT(argv, &i);
                break;
            case 'c':
                count = GET_INT(argv, &i);
                break;
            case 'm':
                magnitude = GET_INT(argv, &i);
                break;
        }
    }
    int fd, ret;
    struct pollfd pfds[1];
    int effects;

    fd = open_event_dev("vibrator", O_RDWR | O_CLOEXEC);
    syscall_error(fd < 0, "Can't open vibrator event device");

    ret = ioctl(fd, EVIOCGEFFECTS, &effects);
    printf("%d effects\n", effects);
    syscall_error(ret < 0, "EVIOCGEFFECTS failed");

    struct ff_effect e = {
            .type = type,
            .id = -1,
            .replay = {
                    .length = length,
                    .delay = delay,
            },
            .u.rumble = {
                    .strong_magnitude = magnitude ,
            },
    };

    ret = ioctl(fd, EVIOCSFF, &e);
    syscall_error(ret < 0, "EVIOCSFF failed");

    struct input_event play = {
            .type = EV_FF,
            .code = e.id,
            .value = count,
    };

    ret = write(fd, &play, sizeof play);
    syscall_error(ret < 0, "write failed");

    int duration = (length+delay)*count;
    sleep(duration / 1000);
    ret = ioctl(fd, EVIOCRMFF, e.id);
    syscall_error(ret < 0, "EVIOCRMFF failed");

    close(fd);
    return 0;
}
