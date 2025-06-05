#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdbool.h>

// Structure to hold a key combination
struct key_combo {
    const char *keys[10];  // Array of key strings
    int count;             // Number of keys in the combination
    const char *action;    // The action this combination represents
};

// Function to interpret a sequence of keys as an action
const char *interpret_key_combo(const char *keys[], int count);

// Function to check if a string is a modifier key
bool is_modifier_key(const char *key);

#endif // KEYMAP_H 