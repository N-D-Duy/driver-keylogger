#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

#include <stddef.h>

// Initialize device client
int device_client_init(const char *device_path);

// Read data from device
int device_client_read(char *buffer, size_t size);

// Cleanup device client
void device_client_cleanup(void);

#endif // DEVICE_CLIENT_H 