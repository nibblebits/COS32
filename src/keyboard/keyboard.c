#include "io/io.h"
void enable_keyboard()
{
    outb(0x64, 0xAE);
}