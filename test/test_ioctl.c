#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_SET_VAL 1

int main(void) {
    int fd = open("/dev/datto_char", O_RDWR);
    int ret = 0;

    if (fd == -1) {
        printf("Device open error, make sure the datto_char device exists, the user has enough priviledge and try sudo\n");
        return -1;
    }

    printf("Please type the char for ioctl and press ENTER:\n");

    char c = getchar();
    ret = ioctl(fd, IOCTL_SET_VAL, c);

    if (ret < 0) {
        printf("ioctl return error %d\n", ret);
    } else {
        printf("ioctl complete\n");
    }

    close(fd);

    return ret;
}

