#ifndef CHIP8_KEYBOARD_H
#define CHIP8_KEYBOARD_H
#include <stdbool.h>
#include "config.h"

/*
	The computers which originally used the Chip-8 Language had a 16-key
	hexadecimal keypad with the following layout:

	1	2	3	C
	4	5	6	D
	7	8	9	E
	A	0	B	F

	This layout must be mapped into various other configurations to fit
	the keyboards of today's platforms.
 */


struct chip8_keyboard
{
	bool keyboard[CHIP_TOTAL_KEYS];
	const char* keyboard_map;
};

void chip8_keyboard_set_map(struct chip8_keyboard* keyboard, const char* map);
int chip8_keyboard_map(const struct chip8_keyboard *keyboard, const int key);
void chip8_keyboard_down(struct chip8_keyboard *keyboard, const int key);
void chip8_keyboard_up(struct chip8_keyboard *keyboard, const int key);
bool chip8_keyboard_is_down(const struct chip8_keyboard* keyboard, const int key);

#endif
