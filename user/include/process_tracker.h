#ifndef PROCESS_TRACKER_H
#define PROCESS_TRACKER_H

#include <stddef.h>

// Structure to hold process information
struct process_info {
    int pid;
    char name[256];
    char cmdline[1024];
    char window_title[512];
    unsigned long last_update;
};

// Initialize process tracker
int process_tracker_init(void);

// Get current active process information
int process_tracker_get_active_process(struct process_info *info);

// Check if active process has changed since last call
int process_tracker_has_changed(void);

// Get current active window title (X11)
int process_tracker_get_window_title(char *title, size_t size);

// Cleanup process tracker
void process_tracker_cleanup(void);

#endif // PROCESS_TRACKER_H 