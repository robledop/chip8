#include "chip8_memory.h"
#include <assert.h>

static void chip8_is_memory_in_bounds(const int index)
{
	assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}

void chip8_memory_set(struct chip8_memory* memory, const int index, const unsigned char val)
{
	chip8_is_memory_in_bounds(index);
	memory->memory[index] = val;
}

unsigned char chip8_memory_get(const struct chip8_memory* memory, const int index)
{
	chip8_is_memory_in_bounds(index);
	return memory->memory[index];
}

unsigned short chip8_memory_get_short(const struct chip8_memory* memory, const int index)
{
	const unsigned char byte1 = chip8_memory_get(memory, index);
	const unsigned char byte2 = chip8_memory_get(memory, index + 1);
	return byte1 << 8 | byte2;
}