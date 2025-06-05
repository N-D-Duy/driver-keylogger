#include <string.h>
#include "keymap.h"

// Common modifier keys
static const char *modifier_keys[] = {
    "[CTRL]", "[LSHIFT]", "[RSHIFT]", "[ALT]", "[CAPS]"
};

// Common key combinations and their actions
static const struct key_combo known_combos[] = {
    {
        .keys = {"[CTRL]", "c"},
        .count = 2,
        .action = "Copy"
    },
    {
        .keys = {"[CTRL]", "v"},
        .count = 2,
        .action = "Paste"
    },
    {
        .keys = {"[CTRL]", "x"},
        .count = 2,
        .action = "Cut"
    },
    {
        .keys = {"[CTRL]", "z"},
        .count = 2,
        .action = "Undo"
    },
    {
        .keys = {"[CTRL]", "y"},
        .count = 2,
        .action = "Redo"
    },
    {
        .keys = {"[CTRL]", "s"},
        .count = 2,
        .action = "Save"
    },
    {
        .keys = {"[CTRL]", "f"},
        .count = 2,
        .action = "Find"
    },
    {
        .keys = {"[CTRL]", "a"},
        .count = 2,
        .action = "Select All"
    },
    {
        .keys = {"[CTRL]", "[F7]"},
        .count = 2,
        .action = "Spell Check"
    },
    {
        .keys = {"[CTRL]", "[F7]", "[F7]"},
        .count = 3,
        .action = "Advanced Spell Check"
    },
    {
        .keys = {"[ALT]", "[TAB]"},
        .count = 2,
        .action = "Switch Window"
    },
    {
        .keys = {"[CTRL]", "[ALT]", "[DELETE]"},
        .count = 3,
        .action = "Task Manager"
    }
};

bool is_modifier_key(const char *key) {
    for (size_t i = 0; i < sizeof(modifier_keys) / sizeof(modifier_keys[0]); i++) {
        if (strcmp(key, modifier_keys[i]) == 0) {
            return true;
        }
    }
    return false;
}

const char *interpret_key_combo(const char *keys[], int count) {
    // Check against known combinations
    for (size_t i = 0; i < sizeof(known_combos) / sizeof(known_combos[0]); i++) {
        if (known_combos[i].count != count) continue;
        
        bool match = true;
        for (int j = 0; j < count; j++) {
            if (strcmp(keys[j], known_combos[i].keys[j]) != 0) {
                match = false;
                break;
            }
        }
        
        if (match) {
            return known_combos[i].action;
        }
    }
    
    // If no exact match, try to make a reasonable guess
    if (count == 2 && is_modifier_key(keys[0])) {
        if (strcmp(keys[0], "[CTRL]") == 0) {
            return "Custom Ctrl+Key Combination";
        } else if (strcmp(keys[0], "[ALT]") == 0) {
            return "Custom Alt+Key Combination";
        } else if (strcmp(keys[0], "[LSHIFT]") == 0 || strcmp(keys[0], "[RSHIFT]") == 0) {
            return "Shifted Key";
        }
    }
    
    return NULL;  // No interpretation available
} 