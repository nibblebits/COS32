#include "classic.h"
#include "memory/memory.h"
#include "memory/idt/idt.h"
#include "io/io.h"
#include "status.h"
#include "kernel.h"


int classic_keyboard_init();
int classic_keyboard_push(int keycode);
int classic_keyboard_pop();

// An array of special key scancodes ( I don't like magic numbers change this soon )
static uint8_t keyboard_special_keys_set_one[] = {
    0x3B, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0xf2, 0x43, 0x44, 0x57, 0x58};

/*
 		0x01 	escape pressed 	0x02 	1 pressed 	0x03 	2 pressed
0x04 	3 pressed 	0x05 	4 pressed 	0x06 	5 pressed 	0x07 	6 pressed
0x08 	7 pressed 	0x09 	8 pressed 	0x0A 	9 pressed 	0x0B 	0 (zero) pressed
0x0C 	- pressed 	0x0D 	= pressed 	0x0E 	backspace pressed 	0x0F 	tab pressed
0x10 	Q pressed 	0x11 	W pressed 	0x12 	E pressed 	0x13 	R pressed
0x14 	T pressed 	0x15 	Y pressed 	0x16 	U pressed 	0x17 	I pressed
0x18 	O pressed 	0x19 	P pressed 	0x1A 	[ pressed 	0x1B 	] pressed
0x1C 	enter pressed 	0x1D 	left control pressed 	0x1E 	A pressed 	0x1F 	S pressed
0x20 	D pressed 	0x21 	F pressed 	0x22 	G pressed 	0x23 	H pressed
0x24 	J pressed 	0x25 	K pressed 	0x26 	L pressed 	0x27 	 ; pressed
0x28 	' (single quote) pressed 	0x29 	` (back tick) pressed 	0x2A 	left shift pressed 	0x2B 	\ pressed
0x2C 	Z pressed 	0x2D 	X pressed 	0x2E 	C pressed 	0x2F 	V pressed
0x30 	B pressed 	0x31 	N pressed 	0x32 	M pressed 	0x33 	, pressed
0x34 	. pressed 	0x35 	/ pressed 	0x36 	right shift pressed 	0x37 	(keypad) * pressed
0x38 	left alt pressed 	0x39 	space pressed 	0x3A 	CapsLock pressed 	0x3B 	F1 pressed
0x3C 	F2 pressed 	0x3D 	F3 pressed 	0x3E 	F4 pressed 	0x3F 	F5 pressed
0x40 	F6 pressed 	0x41 	F7 pressed 	0x42 	F8 pressed 	0x43 	F9 pressed
0x44 	F10 pressed 	0x45 	NumberLock pressed 	0x46 	ScrollLock pressed 	0x47 	(keypad) 7 pressed
0x48 	(keypad) 8 pressed 	0x49 	(keypad) 9 pressed 	0x4A 	(keypad) - pressed 	0x4B 	(keypad) 4 pressed
0x4C 	(keypad) 5 pressed 	0x4D 	(keypad) 6 pressed 	0x4E 	(keypad) + pressed 	0x4F 	(keypad) 1 pressed
0x50 	(keypad) 2 pressed 	0x51 	(keypad) 3 pressed 	0x52 	(keypad) 0 pressed 	0x53 	(keypad) . pressed
						0x57 	F11 pressed
0x58 	F12 pressed 						
		0x81 	escape released 	0x82 	1 released 	0x83 	2 released
0x84 	3 released 	0x85 	4 released 	0x86 	5 released 	0x87 	6 released 
*/
static uint8_t keyboard_scan_set_one[] = {
    0x00,
    0x1B,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    0x08,
    '\t',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '[',
    ']',
    0x0d,
    0x00,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ';',
    '\'',
    '`',
    0x00,
    '\'',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    ',',
    '.',
    '/',
    0X00,
    '*',
    0x00,
    0x20,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    '7',
    '8',
    '9',
    '-',
    '4',
    '5',
    '6',
    '+',
    '1',
    '2',
    '3',
    '0',
    '.',
};
struct keyboard classic_keyboard = {
    .name = {"Classic"},
    .init = classic_keyboard_init,
};

void classic_enable_keyboard()
{
    // Magic numbers are always crap, this needs changing
    outb(0x64, 0xAE);    
}

/**
 * Returns the index in the special array of the special scan code provided.
 * If the scan code is not special -1 is returned
 */
static int classic_keyboard_scancode_special(uint8_t scancode)
{
    size_t total_special_keys = sizeof(keyboard_special_keys_set_one) / sizeof(uint8_t);
    for (size_t i = 0; i < total_special_keys; i++)
    {
        if (keyboard_special_keys_set_one[i] == scancode)
        {
            return i;
        }
    }

    return -1;
}

uint8_t classic_keyboard_scancode_to_char(uint8_t scancode)
{
    size_t size_of_keyboard_set_one = sizeof(keyboard_scan_set_one) / sizeof(uint8_t);
    if (scancode > size_of_keyboard_set_one)
    {
        return 0;
    }

    return keyboard_scan_set_one[scancode];
}

/**
 * Called by the ISR for the classic keyboard
 */
void classic_keyboard_handle_interrupt()
{
    uint8_t scancode = 0;
    scancode = insb(KEYBOARD_INPUT_PORT);
    // Sometimes we have a rouge IRQ, osdev says to do a dummy read
    insb(KEYBOARD_INPUT_PORT);

    // Unset the key released b it as we want to get the special index regardless if the key was released or not
    int special_index = classic_keyboard_scancode_special(scancode & ~CLASSIC_KEYBOARD_KEY_RELEASED);
    
    // Key is released
    if (scancode & CLASSIC_KEYBOARD_KEY_RELEASED)
    {
        // Is the scan code special?
        if (special_index != -1)
        {
            keyboard_special_off(special_index);
        }

        return;
    }

    // Is the scan code special?
    if (special_index != -1)
    {
        keyboard_special_on(special_index);
        return;
    }

    uint8_t c = classic_keyboard_scancode_to_char(scancode);
    if (c != 0)
    {
        keyboard_push(c);
    }
}

int classic_keyboard_init()
{
    // Enable the keyboard on the BUS
    classic_enable_keyboard();

    // Let's create an interrupt listener for the keyboard interrupt
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, classic_keyboard_handle_interrupt);

    print("Classic Keyboard Initialized\n");
    return 0;
}

int classic_keyboard_push(__attribute__((unused))int keycode)
{
    return -EIO;
}

int classic_keyboard_pop()
{
    return -EIO;
}

struct keyboard *classic_init()
{
    return &classic_keyboard;
}