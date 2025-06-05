#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "logkey"

static int major;
static char msg[256] = "Hello from kernel!\n";
static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset) {
    return simple_read_from_buffer(buffer, len, offset, msg, strlen(msg));
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
};

static int __init logkey_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) return major;
    pr_info("logkey loaded. Major: %d\n", major);
    return 0;
}

static void __exit logkey_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("logkey unloaded.\n");
}

module_init(logkey_init);
module_exit(logkey_exit);
MODULE_LICENSE("GPL");
