#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

int main(void) {
    const int buf_size = 2560;
    char out_buf[buf_size];
    int ret = 0;

    int fd = open("/dev/datto_char", O_RDWR);
    if (fd < 0) {
        printf("Device open error, make sure the datto_char device exists, the user has enough priviledge and try sudo\n");
        ret = -1;
        return ret;
    }

    int read_len = 0;
    while (true) {
        memset(out_buf, 0, buf_size*sizeof(char));
        read_len = read(fd, out_buf, buf_size*sizeof(char));
        if (read_len < 0) {
            printf("run test_ioctl to set the char to the driver\n");
            ret = -2;
            break;
        } else {
           printf("%s", out_buf);
        }

        usleep(50000);
    }

    close(fd);

    return ret;
}

