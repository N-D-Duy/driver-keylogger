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
    
    if (keycode >= 0xF000 && keycode <= 0xFFFF) {
        switch (keycode) {
            case 0xF601: strncpy(buffer, "[LEFT]", buff_size - 1); break;
            case 0xF602: strncpy(buffer, "[RIGHT]", buff_size - 1); break;
            case 0xF603: strncpy(buffer, "[UP]", buff_size - 1); break;
            case 0xF600: strncpy(buffer, "[DOWN]", buff_size - 1); break;
            case 0xF118: strncpy(buffer, "[PAGE_UP]", buff_size - 1); break;
            case 0xF119: strncpy(buffer, "[PAGE_DOWN]", buff_size - 1); break;
            case 0xF114: strncpy(buffer, "[HOME]", buff_size - 1); break;
            case 0xF117: strncpy(buffer, "[END]", buff_size - 1); break;
            case 0xF115: strncpy(buffer, "[INSERT]", buff_size - 1); break;
            case 0xF116: strncpy(buffer, "[DELETE]", buff_size - 1); break;
            // Handle FB00-FBFF range (lowercase letters)
            case 0xFB61: strncpy(buffer, "a", buff_size - 1); break;
            case 0xFB62: strncpy(buffer, "b", buff_size - 1); break;
            case 0xFB63: strncpy(buffer, "c", buff_size - 1); break;
            case 0xFB64: strncpy(buffer, "d", buff_size - 1); break;
            case 0xFB65: strncpy(buffer, "e", buff_size - 1); break;
            case 0xFB66: strncpy(buffer, "f", buff_size - 1); break;
            case 0xFB67: strncpy(buffer, "g", buff_size - 1); break;
            case 0xFB68: strncpy(buffer, "h", buff_size - 1); break;
            case 0xFB69: strncpy(buffer, "i", buff_size - 1); break;
            case 0xFB6A: strncpy(buffer, "j", buff_size - 1); break;
            case 0xFB6B: strncpy(buffer, "k", buff_size - 1); break;
            case 0xFB6C: strncpy(buffer, "l", buff_size - 1); break;
            case 0xFB6D: strncpy(buffer, "m", buff_size - 1); break;
            case 0xFB6E: strncpy(buffer, "n", buff_size - 1); break;
            case 0xFB6F: strncpy(buffer, "o", buff_size - 1); break;
            case 0xFB70: strncpy(buffer, "p", buff_size - 1); break;
            case 0xFB71: strncpy(buffer, "q", buff_size - 1); break;
            case 0xFB72: strncpy(buffer, "r", buff_size - 1); break;
            case 0xFB73: strncpy(buffer, "s", buff_size - 1); break;
            case 0xFB74: strncpy(buffer, "t", buff_size - 1); break;
            case 0xFB75: strncpy(buffer, "u", buff_size - 1); break;
            case 0xFB76: strncpy(buffer, "v", buff_size - 1); break;
            case 0xFB77: strncpy(buffer, "w", buff_size - 1); break;
            case 0xFB78: strncpy(buffer, "x", buff_size - 1); break;
            case 0xFB79: strncpy(buffer, "y", buff_size - 1); break;
            case 0xFB7A: strncpy(buffer, "z", buff_size - 1); break;
            case 0xF020: strncpy(buffer, " ", buff_size - 1); break;  // Space
            case 0xF201: strncpy(buffer, "[ENTER]", buff_size - 1); break;  // Enter
            case 0xF07F: strncpy(buffer, "[BACKSPACE]", buff_size - 1); break;  // Backspace
            case 0xF009: strncpy(buffer, "[TAB]", buff_size - 1); break;  // Tab
            case 0xF01B: strncpy(buffer, "[ESC]", buff_size - 1); break;  // Escape
            case 0xF702: strncpy(buffer, "[CTRL]", buff_size - 1); break;  // Ctrl
            case 0xF700: strncpy(buffer, "[ALT]", buff_size - 1); break;  // Alt
            case 0xFA06: strncpy(buffer, "[CAPS]", buff_size - 1); break;  // Caps
            // F1-F12 keys (different range)
            case 0xF100: strncpy(buffer, "[F1]", buff_size - 1); break;
            case 0xF101: strncpy(buffer, "[F2]", buff_size - 1); break;
            case 0xF102: strncpy(buffer, "[F3]", buff_size - 1); break;
            case 0xF103: strncpy(buffer, "[F4]", buff_size - 1); break;
            case 0xF104: strncpy(buffer, "[F5]", buff_size - 1); break;
            case 0xF105: strncpy(buffer, "[F6]", buff_size - 1); break;
            case 0xF106: strncpy(buffer, "[F7]", buff_size - 1); break;
            case 0xF107: strncpy(buffer, "[F8]", buff_size - 1); break;
            case 0xF108: strncpy(buffer, "[F9]", buff_size - 1); break;
            case 0xF109: strncpy(buffer, "[F10]", buff_size - 1); break;
            case 0xF10A: strncpy(buffer, "[F11]", buff_size - 1); break;
            case 0xF10B: strncpy(buffer, "[F12]", buff_size - 1); break;

            // number
            case 0xF030: strncpy(buffer, "0", buff_size - 1); break;
            case 0xF031: strncpy(buffer, "1", buff_size - 1); break;
            case 0xF032: strncpy(buffer, "2", buff_size - 1); break;
            case 0xF033: strncpy(buffer, "3", buff_size - 1); break;
            case 0xF034: strncpy(buffer, "4", buff_size - 1); break;
            case 0xF035: strncpy(buffer, "5", buff_size - 1); break;
            case 0xF036: strncpy(buffer, "6", buff_size - 1); break;
            case 0xF037: strncpy(buffer, "7", buff_size - 1); break;
            case 0xF038: strncpy(buffer, "8", buff_size - 1); break;
            case 0xF039: strncpy(buffer, "9", buff_size - 1); break;
            default: 
                snprintf(buffer, buff_size, "[KEY_%X]", keycode);
                break;
        }
        printk(KERN_INFO "Keymap: Converting special keycode 0x%X to '%s'\n", keycode, buffer);
        return strlen(buffer);
    }
    
    if (keycode > 0 && keycode < ARRAY_SIZE(us_keymap)) {
        if (keycode == 42 && shift) { 
            return 0;
        }
        if (keycode == 54 && shift) { 
            return 0;
        }
        
        const char *s = us_keymap[keycode][shift ? 1 : 0];
        strncpy(buffer, s, buff_size - 1);
        printk(KERN_INFO "Keymap: Converting keycode %d (shift=%d) to '%s'\n", 
               keycode, shift, buffer);
        return strlen(buffer);
    }
    
    printk(KERN_INFO "Keymap: Unknown keycode %d\n", keycode);
    return 0;
}
