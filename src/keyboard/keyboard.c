#include "keyboard.h"
#include "io/io.h"
void enable_keyboard()
{
    outb(0x64, 0xAE);
}

bool keyboard_is_function_key(int key)
{
    return key >= F1_PRESSED && key <= F12_PRESSED;
}