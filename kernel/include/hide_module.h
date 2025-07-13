#ifndef HIDE_MODULE_H
#define HIDE_MODULE_H

#include <linux/module.h>

// Initialize module hiding
int hide_module_init(struct module *mod);

// Cleanup module hiding
void hide_module_exit(void);

#endif // HIDE_MODULE_H 