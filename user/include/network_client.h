#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <stdbool.h>
#include <stddef.h>

int network_client_init(const char *config_file);

int network_client_send(const char *data, size_t len);

void network_client_cleanup(void);

#endif // NETWORK_CLIENT_H 