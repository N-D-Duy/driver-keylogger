#include <linux/module.h>
#include "keylogger.h"

static int __init logkey_init(void) {
    int ret;
    
    ret = keylogger_init();
    if (ret < 0) {
        return ret;
    }
    
    return 0;
}

static void __exit logkey_exit(void) {
    keylogger_exit();
}

module_init(logkey_init);
module_exit(logkey_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Duy Nguyen");
MODULE_DESCRIPTION("Keyboard logger using character device"); 