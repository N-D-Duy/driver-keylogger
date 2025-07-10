#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "device_client.h"

static int device_fd = -1;

int device_client_init(const char *device_path) {
    device_fd = open(device_path, O_RDONLY);
    if (device_fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    
    printf("Opened device: %s\n", device_path);
    return 0;
}

int device_client_read(char *buffer, size_t size) {
    if (device_fd < 0) {
        return -1;
    }

    ssize_t bytes_read = read(device_fd, buffer, size - 1);
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // No data available, return 0
            return 0;
        }
        perror("Failed to read from device");
        return -1;
    }

    return bytes_read;
}

void device_client_cleanup(void) {
    if (device_fd >= 0) {
        close(device_fd);
        device_fd = -1;
    }
} 