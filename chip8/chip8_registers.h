#ifndef CHIP8_REGISTERS_H
#define  CHIP8_REGISTERS_H
#include "config.h"

/*
	REGISTERS
	
	Chip-8 has 16 general purpose 8-bit registers, usually referred to as Vx,
	where x is a hexadecimal digit (0 through F). There is also a 16-bit register
	called I. This register is generally used to store memory addresses, so only
	the lowest (rightmost) 12 bits are usually used.

	The VF register should not be used by any program, as it is used as a flag by
	some instructions. See section 3.0, Instructions for details.

	Chip-8 also has two special purpose 8-bit registers, for the delay and sound timers.
	When these registers are non-zero, they are automatically decremented at a rate of 60Hz.
	See the section 2.5, Timers & Sound, for more information on these.

	There are also some "pseudo-registers" which are not accessible from Chip-8 programs.
	The program counter (PC) should be 16-bit, and is used to store the currently executing
	address. The stack pointer (SP) can be 8-bit, it is used to point to the
	topmost level of the stack.
*/


struct chip8_registers
{
	// 16 general purpose 8-bit registers
	unsigned char V[CHIP8_TOTAL_DATA_REGISTERS];
	// 16-bit general purpose register
	unsigned short I;
	unsigned char delay_timer;
	unsigned char sound_timer;
	// Program counter
	unsigned short PC;
	// Stack pointer
	unsigned char SP;
};


#endif

