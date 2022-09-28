#include <assert.h>
#include "chip8_keyboard.h"

static void chip8_keyboard_is_in_bounds(const int key)
{
	assert(key >= 0 && key < CHIP_TOTAL_KEYS);
}

void chip8_keyboard_set_map(struct chip8_keyboard* keyboard, const char* map)
{
	keyboard->keyboard_map = map;
}

int chip8_keyboard_map(const struct chip8_keyboard *keyboard, const int key)
{
	for (int i = 0; i < CHIP_TOTAL_KEYS; i++)
	{
		if (keyboard->keyboard_map[i] == key)
		{
			return i;
		}
	}

	return -1;
}

void chip8_keyboard_down(struct chip8_keyboard *keyboard, const int key)
{
	chip8_keyboard_is_in_bounds(key);
	keyboard->keyboard[key] = true;
}

void chip8_keyboard_up(struct chip8_keyboard *keyboard, const int key)
{
	chip8_keyboard_is_in_bounds(key);
	keyboard->keyboard[key] = false;
}

bool chip8_keyboard_is_down(const struct chip8_keyboard *keyboard, const int key)
{

	chip8_keyboard_is_in_bounds(key);
	return keyboard->keyboard[key];
}