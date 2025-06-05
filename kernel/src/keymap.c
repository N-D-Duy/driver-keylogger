#include <linux/string.h>
#include <linux/kernel.h>
#include "keymap.h"

static const char *us_keymap[][2] = {
    {"\0", "\0"},     // 0
    {"[ESC]", "[ESC]"}, // 1
    {"1", "!"},       // 2
    {"2", "@"},       // 3
    {"3", "#"},       // 4
    {"4", "$"},       // 5
    {"5", "%"},       // 6
    {"6", "^"},       // 7
    {"7", "&"},       // 8
    {"8", "*"},       // 9
    {"9", "("},       // 10
    {"0", ")"},       // 11
    {"-", "_"},       // 12
    {"=", "+"},       // 13
    {"[BACKSPACE]", "[BACKSPACE]"}, // 14
    {"[TAB]", "[TAB]"}, // 15
    {"q", "Q"},       // 16
    {"w", "W"},       // 17
    {"e", "E"},       // 18
    {"r", "R"},       // 19
    {"t", "T"},       // 20
    {"y", "Y"},       // 21
    {"u", "U"},       // 22
    {"i", "I"},       // 23
    {"o", "O"},       // 24
    {"p", "P"},       // 25
    {"[", "{"},       // 26
    {"]", "}"},       // 27
    {"[ENTER]", "[ENTER]"}, // 28
    {"[CTRL]", "[CTRL]"}, // 29
    {"a", "A"},       // 30
    {"s", "S"},       // 31
    {"d", "D"},       // 32
    {"f", "F"},       // 33
    {"g", "G"},       // 34
    {"h", "H"},       // 35
    {"j", "J"},       // 36
    {"k", "K"},       // 37
    {"l", "L"},       // 38
    {";", ":"},       // 39
    {"'", "\""},      // 40
    {"`", "~"},       // 41
    {"[LSHIFT]", "[LSHIFT]"}, // 42
    {"\\", "|"},      // 43
    {"z", "Z"},       // 44
    {"x", "X"},       // 45
    {"c", "C"},       // 46
    {"v", "V"},       // 47
    {"b", "B"},       // 48
    {"n", "N"},       // 49
    {"m", "M"},       // 50
    {",", "<"},       // 51
    {".", ">"},       // 52
    {"/", "?"},       // 53
    {"[RSHIFT]", "[RSHIFT]"}, // 54
    {"[PRINT]", "[PRINT]"}, // 55
    {"[ALT]", "[ALT]"}, // 56
    {" ", " "},       // 57
    {"[CAPS]", "[CAPS]"}, // 58
};

size_t keycode_to_us_string(int keycode, int shift, char *buffer, size_t buff_size) {
    memset(buffer, 0, buff_size);
    
    // Handle special keycodes
    if (keycode >= 0xE000) {  // Special keycodes start at 0xE000
        switch (keycode) {
            case 0xE000: strncpy(buffer, "[LEFT]", buff_size - 1); break;
            case 0xE001: strncpy(buffer, "[RIGHT]", buff_size - 1); break;
            case 0xE002: strncpy(buffer, "[UP]", buff_size - 1); break;
            case 0xE003: strncpy(buffer, "[DOWN]", buff_size - 1); break;
            case 0xE004: strncpy(buffer, "[PAGE_UP]", buff_size - 1); break;
            case 0xE005: strncpy(buffer, "[PAGE_DOWN]", buff_size - 1); break;
            case 0xE006: strncpy(buffer, "[HOME]", buff_size - 1); break;
            case 0xE007: strncpy(buffer, "[END]", buff_size - 1); break;
            case 0xE008: strncpy(buffer, "[INSERT]", buff_size - 1); break;
            case 0xE009: strncpy(buffer, "[DELETE]", buff_size - 1); break;
            case 0xE00A: strncpy(buffer, "[F1]", buff_size - 1); break;
            case 0xE00B: strncpy(buffer, "[F2]", buff_size - 1); break;
            case 0xE00C: strncpy(buffer, "[F3]", buff_size - 1); break;
            case 0xE00D: strncpy(buffer, "[F4]", buff_size - 1); break;
            case 0xE00E: strncpy(buffer, "[F5]", buff_size - 1); break;
            case 0xE00F: strncpy(buffer, "[F6]", buff_size - 1); break;
            case 0xE010: strncpy(buffer, "[F7]", buff_size - 1); break;
            case 0xE011: strncpy(buffer, "[F8]", buff_size - 1); break;
            case 0xE012: strncpy(buffer, "[F9]", buff_size - 1); break;
            case 0xE013: strncpy(buffer, "[F10]", buff_size - 1); break;
            case 0xE014: strncpy(buffer, "[F11]", buff_size - 1); break;
            case 0xE015: strncpy(buffer, "[F12]", buff_size - 1); break;
            default: 
                snprintf(buffer, buff_size, "[KEY_%X]", keycode);
                break;
        }
        printk(KERN_INFO "Keymap: Converting special keycode 0x%X to '%s'\n", keycode, buffer);
        return strlen(buffer);
    }
    
    if (keycode > 0 && keycode < ARRAY_SIZE(us_keymap)) {
        const char *s = us_keymap[keycode][shift ? 1 : 0];
        strncpy(buffer, s, buff_size - 1);
        printk(KERN_INFO "Keymap: Converting keycode %d (shift=%d) to '%s'\n", 
               keycode, shift, buffer);
        return strlen(buffer);
    }
    
    printk(KERN_INFO "Keymap: Unknown keycode %d\n", keycode);
    return 0;
}
