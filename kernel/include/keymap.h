#ifndef KEYMAP_H
#define KEYMAP_H

#include <linux/types.h>

size_t keycode_to_us_string(int keycode, int shift, char *buffer, size_t buff_size);

#endif // KEYMAP_H
