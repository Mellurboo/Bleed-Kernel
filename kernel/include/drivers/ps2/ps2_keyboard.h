#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include <stdint.h>

typedef void (*keyboard_callback_t)(char c);

void ps2_keyboard_irq(uint8_t irq);
void keyboard_set_callback(keyboard_callback_t cb);

static const char keymap[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a',
    's','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c',
    'v','b','n','m',',','.','/',0,'*',0,' ',0
};

// im british, this is a british kernel, born and bred. we will use the british keyboard
static const char keymap_shift[128] = {
    0,27,'!','"','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A',
    'S','D','F','G','H','J','K','L',':','@','~',0,'|','Z','X','C',
    'V','B','N','M','<','>','?',0,'*',0,' ',0
};

#endif