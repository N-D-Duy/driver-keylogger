#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include "network_client.h"

// TCP keepalive options (may not be defined on all systems)
#ifndef TCP_KEEPIDLE
#define TCP_KEEPIDLE 4
#endif
#ifndef TCP_KEEPINTVL
#define TCP_KEEPINTVL 5
#endif
#ifndef TCP_KEEPCNT
#define TCP_KEEPCNT 6
#endif

#define MAX_LINE_LENGTH 256

static int sock_fd = -1;
static char server_host[256];
static int server_port;

static int read_config(const char *config_file) {
    FILE *fp = fopen(config_file, "r");
    if (!fp) {
        perror("Failed to open config file");
        return -1;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp)) {
        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        if (strncmp(line, "SERVER_HOST=", 12) == 0) {
            char *host = line + 12;
            // Remove tcp:// prefix if present
            if (strncmp(host, "tcp://", 6) == 0) {
                host += 6;
            }
            strncpy(server_host, host, sizeof(server_host) - 1);
            server_host[sizeof(server_host) - 1] = '\0';
        } else if (strncmp(line, "SERVER_PORT=", 12) == 0) {
            server_port = atoi(line + 12);
        }
    }

    fclose(fp);
    return 0;
}

int network_client_init(const char *config_file) {
    if (read_config(config_file) < 0) {
        return -1;
    }

    // Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Failed to create socket");
        return -1;
    }

    // Set socket options for better connection stability
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0) {
        perror("Failed to set SO_KEEPALIVE");
    }
    
    // Enable TCP keepalive
    if (setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPIDLE, &(int){60}, sizeof(int)) < 0) {
        perror("Failed to set TCP_KEEPIDLE");
    }
    
    if (setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPINTVL, &(int){10}, sizeof(int)) < 0) {
        perror("Failed to set TCP_KEEPINTVL");
    }
    
    if (setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPCNT, &(int){3}, sizeof(int)) < 0) {
        perror("Failed to set TCP_KEEPCNT");
    }

    // Set send timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Failed to set SO_SNDTIMEO");
    }

    // Resolve hostname to IP address
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(server_host, NULL, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        close(sock_fd);
        return -1;
    }

    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;

    freeaddrinfo(result);

    // Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock_fd);
        return -1;
    }

    printf("Connected to server %s:%d\n", server_host, server_port);
    return 0;
}

int network_client_send(const char *data, size_t len) {
    if (sock_fd < 0) {
        return -1;
    }

    // Check if socket is still valid
    int error = 0;
    socklen_t len_error = sizeof(error);
    if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, &len_error) < 0 || error != 0) {
        fprintf(stderr, "Socket error detected: %s\n", strerror(error));
        return -1;
    }

    ssize_t sent = send(sock_fd, data, len, MSG_NOSIGNAL);
    if (sent < 0) {
        if (errno == EPIPE || errno == ECONNRESET) {
            fprintf(stderr, "Connection lost: %s\n", strerror(errno));
        } else {
            perror("Failed to send data");
        }
        return -1;
    }

    return sent;
}

void network_client_cleanup(void) {
    if (sock_fd >= 0) {
        close(sock_fd);
        sock_fd = -1;
    }
} 