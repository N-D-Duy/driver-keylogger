#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "process_tracker.h"

static struct process_info current_process = {0};
static struct process_info last_process = {0};
static Display *display = NULL;
static Window last_active_window = 0;

int process_tracker_init(void) {
    // Initialize X11 display for window tracking
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Warning: Could not open X11 display. Window tracking disabled.\n");
    }
    
    memset(&current_process, 0, sizeof(current_process));
    memset(&last_process, 0, sizeof(last_process));
    last_active_window = 0;
    
    return 0;
}

static int read_process_name(int pid, char *name, size_t size) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    
    if (fgets(name, size, fp)) {
        char *newline = strchr(name, '\n');
        if (newline) *newline = '\0';
    } else {
        fclose(fp);
        return -1;
    }
    
    fclose(fp);
    return 0;
}

static int read_process_cmdline(int pid, char *cmdline, size_t size) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    
    size_t bytes_read = fread(cmdline, 1, size - 1, fp);
    cmdline[bytes_read] = '\0';
    
    // Replace null bytes with spaces for readability
    for (size_t i = 0; i < bytes_read; i++) {
        if (cmdline[i] == '\0') {
            cmdline[i] = ' ';
        }
    }
    
    fclose(fp);
    return 0;
}

static int get_pid_from_window(Window window) {
    if (!display || window == 0) {
        return -1;
    }
    
    Atom pid_atom = XInternAtom(display, "_NET_WM_PID", False);
    if (pid_atom == None) {
        return -1;
    }
    
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    
    if (XGetWindowProperty(display, window, pid_atom, 0, 1, False, 
                          AnyPropertyType, &actual_type, &actual_format, 
                          &nitems, &bytes_after, &prop) == Success && prop) {
        int pid = *(int*)prop;
        XFree(prop);
        return pid;
    }
    
    return -1;
}

int process_tracker_get_window_title(char *title, size_t size) {
    if (!display) {
        strncpy(title, "No X11 Display", size - 1);
        title[size - 1] = '\0';
        return -1;
    }
    
    Window root = DefaultRootWindow(display);
    Atom atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (atom == None) {
        strncpy(title, "No EWMH Support", size - 1);
        title[size - 1] = '\0';
        return -1;
    }
    
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;
    Window active_window = 0;

    int result = XGetWindowProperty(display, root, atom, 0, (~0L), False, AnyPropertyType,
                                   &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    
    if (result == Success && prop) {
        active_window = *(Window *)prop;
        XFree(prop);
    }

    if (active_window) {
        // Try multiple methods to get window name
        char *window_name = NULL;
        
        // Method 1: XFetchName (WM_NAME)
        if (XFetchName(display, active_window, &window_name) && window_name) {
            strncpy(title, window_name, size - 1);
            title[size - 1] = '\0';
            XFree(window_name);
            return 0;
        }
        
        // Method 2: XGetWMName (WM_NAME as property)
        XTextProperty text_prop;
        if (XGetWMName(display, active_window, &text_prop) && text_prop.value) {
            strncpy(title, (char*)text_prop.value, size - 1);
            title[size - 1] = '\0';
            XFree(text_prop.value);
            return 0;
        }
        
        // Method 3: Try to get _NET_WM_NAME (EWMH)
        Atom wm_name_atom = XInternAtom(display, "_NET_WM_NAME", False);
        if (wm_name_atom != None) {
            unsigned char *prop = NULL;
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            
            if (XGetWindowProperty(display, active_window, wm_name_atom, 0, (~0L), False, 
                                  AnyPropertyType, &actual_type, &actual_format, 
                                  &nitems, &bytes_after, &prop) == Success && prop) {
                strncpy(title, (char*)prop, size - 1);
                title[size - 1] = '\0';
                XFree(prop);
                return 0;
            }
        }
        
        // Method 4: Try to get _NET_WM_VISIBLE_NAME (for some applications)
        Atom visible_name_atom = XInternAtom(display, "_NET_WM_VISIBLE_NAME", False);
        if (visible_name_atom != None) {
            unsigned char *prop = NULL;
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            
            if (XGetWindowProperty(display, active_window, visible_name_atom, 0, (~0L), False, 
                                  AnyPropertyType, &actual_type, &actual_format, 
                                  &nitems, &bytes_after, &prop) == Success && prop) {
                strncpy(title, (char*)prop, size - 1);
                title[size - 1] = '\0';
                XFree(prop);
                return 0;
            }
        }
    }

    // Fallback: try XGetInputFocus
    Window focused;
    int revert_to;
    XGetInputFocus(display, &focused, &revert_to);
    
    if (focused != None && focused != PointerRoot) {
        char *window_name = NULL;
        if (XFetchName(display, focused, &window_name) && window_name) {
            strncpy(title, window_name, size - 1);
            title[size - 1] = '\0';
            XFree(window_name);
            return 0;
        }
    }

    // If all else fails, return error to let caller handle fallback
    return -1;
}

int process_tracker_get_active_process(struct process_info *info) {
    if (!info) {
        return -1;
    }
    
    int fg_pid = -1;
    Window active_window = 0;
    
    // Try to get PID from active window first (most reliable for GUI apps)
    if (display) {
        Window root = DefaultRootWindow(display);
        Atom atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
        
        if (atom != None) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            unsigned char *prop = NULL;

            int result = XGetWindowProperty(display, root, atom, 0, (~0L), False, AnyPropertyType,
                                           &actual_type, &actual_format, &nitems, &bytes_after, &prop);
            
            if (result == Success && prop) {
                active_window = *(Window *)prop;
                XFree(prop);
                
                if (active_window != 0) {
                    fg_pid = get_pid_from_window(active_window);
                }
            }
        }
    }
    
    // Fallback: try to get foreground process group
    if (fg_pid < 0) {
        fg_pid = tcgetpgrp(STDIN_FILENO);
    }
    
    // If still no PID, try to find the most recently active process
    if (fg_pid < 0) {
        DIR *proc_dir = opendir("/proc");
        if (!proc_dir) {
            return -1;
        }
        
        // Find the most recently accessed process (simplified approach)
        struct dirent *entry;
        time_t latest_time = 0;
        int latest_pid = -1;
        
        while ((entry = readdir(proc_dir)) != NULL) {
            if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
                int pid = atoi(entry->d_name);
                char path[256];
                snprintf(path, sizeof(path), "/proc/%d", pid);
                
                struct stat st;
                if (stat(path, &st) == 0 && st.st_atime > latest_time) {
                    latest_time = st.st_atime;
                    latest_pid = pid;
                }
            }
        }
        closedir(proc_dir);
        
        if (latest_pid > 0) {
            fg_pid = latest_pid;
        } else {
            return -1;
        }
    }
    
    // Update current process info
    current_process.pid = fg_pid;
    current_process.last_update = time(NULL);
    
    // Read process name
    if (read_process_name(fg_pid, current_process.name, sizeof(current_process.name)) < 0) {
        strcpy(current_process.name, "unknown");
    }
    
    // Read command line
    if (read_process_cmdline(fg_pid, current_process.cmdline, sizeof(current_process.cmdline)) < 0) {
        strcpy(current_process.cmdline, "unknown");
    }
    
    // Get window title with retry mechanism for clicked applications
    int title_retry_count = 0;
    const int max_retries = 3;
    
    while (title_retry_count < max_retries) {
        if (process_tracker_get_window_title(current_process.window_title, 
                                            sizeof(current_process.window_title)) == 0) {
            // Successfully got window title
            break;
        }
        
        // If we have an active window but failed to get title, try again
        if (active_window != 0 && title_retry_count < max_retries - 1) {
            usleep(100000); // Wait 100ms before retry
            title_retry_count++;
            continue;
        }
        
        // Fallback to process name if window title fails
        if (strcmp(current_process.name, "unknown") != 0) {
            // Create a more descriptive fallback based on process name
            if (strstr(current_process.name, "gnome-terminal") || 
                strstr(current_process.name, "xterm") ||
                strstr(current_process.name, "konsole") ||
                strstr(current_process.name, "terminator")) {
                snprintf(current_process.window_title, sizeof(current_process.window_title), 
                        "Terminal - %s", current_process.name);
            } else if (strstr(current_process.name, "firefox")) {
                snprintf(current_process.window_title, sizeof(current_process.window_title), 
                        "Firefox Browser");
            } else if (strstr(current_process.name, "chrome") || 
                       strstr(current_process.name, "chromium")) {
                snprintf(current_process.window_title, sizeof(current_process.window_title), 
                        "Chrome Browser");
            } else {
                snprintf(current_process.window_title, sizeof(current_process.window_title), 
                        "%s (Window)", current_process.name);
            }
        } else {
            strcpy(current_process.window_title, "unknown");
        }
        break;
    }
    
    // Copy to output
    memcpy(info, &current_process, sizeof(struct process_info));
    
    return 0;
}

int process_tracker_has_changed(void) {
    struct process_info current;
    if (process_tracker_get_active_process(&current) < 0) {
        return 0;
    }
    
    // Check if window has changed (most reliable indicator)
    Window current_active_window = 0;
    if (display) {
        Window root = DefaultRootWindow(display);
        Atom atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
        
        if (atom != None) {
            Atom actual_type;
            int actual_format;
            unsigned long nitems, bytes_after;
            unsigned char *prop = NULL;

            int result = XGetWindowProperty(display, root, atom, 0, (~0L), False, AnyPropertyType,
                                           &actual_type, &actual_format, &nitems, &bytes_after, &prop);
            
            if (result == Success && prop) {
                current_active_window = *(Window *)prop;
                XFree(prop);
            }
        }
    }
    
    // Compare with last known process and window
    int changed = 0;
    
    // Check if PID changed
    if (last_process.pid != current.pid) {
        changed = 1;
    }
    
    // Check if process name changed
    if (strcmp(last_process.name, current.name) != 0) {
        changed = 1;
    }
    
    // Check if window changed (most sensitive indicator)
    if (current_active_window != last_active_window) {
        changed = 1;
    }
    
    // Check if window title changed (for same window but different content)
    if (strcmp(last_process.window_title, current.window_title) != 0) {
        changed = 1;
    }
    
    // Update last process and window
    if (changed) {
        memcpy(&last_process, &current, sizeof(struct process_info));
        last_active_window = current_active_window;
    }
    
    return changed;
}

void process_tracker_cleanup(void) {
    if (display) {
        XCloseDisplay(display);
        display = NULL;
    }
} 