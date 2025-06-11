#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "network_client.h"

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
            strncpy(server_host, line + 12, sizeof(server_host) - 1);
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

    // Set up server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    
    if (inet_pton(AF_INET, server_host, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock_fd);
        return -1;
    }

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

    ssize_t sent = send(sock_fd, data, len, 0);
    if (sent < 0) {
        perror("Failed to send data");
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