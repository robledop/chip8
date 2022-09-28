#ifndef CHIP8_STACK_H
#define CHIP8_STACK_H

#include "config.h"

/*
	The stack is an array of 16 16-bit values, used to store the address that the
	interpreter should return to when finished with a subroutine. Chip-8 allows for
	up to 16 levels of nested subroutines.
 */

struct chip8;
struct chip8_stack
{
	unsigned short stack[CHIP8_TOTAL_STACK_DEPTH];
};

void chip8_stack_push(struct chip8* chip8, unsigned short val);
unsigned short chip8_stack_pop(struct chip8* chip8);

#endif

