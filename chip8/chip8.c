// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticGnuBinaryLiteral
#include <memory.h>
#include "chip8.h"
#include <assert.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>

/*
	The original implementation of the Chip-8 language includes 36 different instructions,
	including math, graphics, and flow control functions.

	Super Chip-48 added an additional 10 instructions, for a total of 46.
	All instructions are 2 bytes long and are stored most-significant-byte first.

	In memory, the first byte of each instruction should be located at an even addresses.
	If a program includes sprite data, it should be padded so any instructions following it
	will be properly situated in RAM.

	This document does not yet contain descriptions of the Super Chip-48 instructions.	They are,
	however, listed below.
	In these listings, the following variables are used:

	nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
	n or nibble - A 4-bit value, the lowest 4 bits of the instruction
	x - A 4-bit value, the lower 4 bits of the high byte of the instruction
	y - A 4-bit value, the upper 4 bits of the low byte of the instruction
	kk or byte - An 8-bit value, the lowest 8 bits of the instruction
 */



 // http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#font
const char chip8_default_character_set[] = {
	0xF0,0x90,0x90,0x90,0xF0,	// 0
	0x20,0x60,0x20,0x20,0x70,	// 1
	0xF0,0x10,0xF0,0x80,0xF0,	// 2
	0xF0,0x10,0xF0,0x10,0xF0,	// 3
	0x90,0x90,0xF0,0x10,0x10,	// 4
	0xF0,0x80,0xF0,0x10,0xF0,	// 5
	0xF0,0x80,0xF0,0x90,0xF0,	// 6
	0xF0,0x10,0x20,0x40,0x40,	// 7
	0xF0,0x90,0xF0,0x90,0xF0,	// 8
	0xF0,0x90,0xF0,0x10,0xF0,	// 9
	0xF0,0x90,0xF0,0x90,0x90,	// A
	0xE0,0x90,0xE0,0x90,0xE0,	// B
	0xF0,0x80,0x80,0x80,0xF0,	// C
	0xE0,0x90,0x90,0x90,0xE0,	// D
	0xF0,0x80,0xF0,0x80,0xF0,	// E
	0xF0,0x80,0xF0,0x80,0x80	// F
};

void chip8_init(struct chip8* chip8)
{
	memset(chip8, 0, sizeof(struct chip8));
	memcpy(&chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

static char chip8_wait_for_key_press(const struct chip8* chip8)
{
	printf("%s\n", "Waiting for a key to be pressed");
	SDL_Event event;
	while (SDL_WaitEvent(&event))
	{
		if (event.type != SDL_KEYDOWN)
		{
			continue;
		}

		const char c = event.key.keysym.sym;
		const char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);

		printf("The key %c has been pressed\n", chip8_key);

		if (chip8_key != -1)
		{
			return chip8_key;
		}
	}

	return  -1;
}

static void chip8_exec_extended(struct chip8* chip8, const unsigned short opcode)
{
	const unsigned short nnn = opcode & 0x0FFF;
	const unsigned short kk = opcode & 0x00FF;
	const unsigned short n = opcode & 0x000F;
	const unsigned short x = (opcode >> 8) & 0x000F;
	const unsigned short y = (opcode >> 4) & 0x000F;

	// the first 4 bits represent the opcode
	switch (opcode & 0xF000)
	{
		// 1nnn - JP addr
		// Jump to location nnn.
		// The interpreter sets the program counter to nnn.
		case 0x1000:
			chip8->registers.PC = nnn;
			break;

			// 2nnn - CALL addr
			// Call subroutine at nnn.
			// The interpreter increments the stack pointer, then puts the current PC on the top of the stack.The PC is then set to nnn.
		case 0x2000:
			chip8_stack_push(chip8, chip8->registers.PC);
			chip8->registers.PC = nnn;
			break;

			// 3xkk - SE Vx, byte
			// Skip next instruction if Vx = kk.
			// The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
		case 0x3000:
			if (chip8->registers.V[x] == kk)
			{
				chip8->registers.PC += 2;
			}
			break;

			// 4xkk - SNE Vx, byte
			// Skip next instruction if Vx != kk.
			// The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
		case 0x4000:
			if (chip8->registers.V[x] != kk)
			{
				chip8->registers.PC += 2;
			}
			break;

			// 5xy0 - SE Vx, Vy
			// Skip next instruction if Vx = Vy.
			// The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
		case 0x5000:
			if (chip8->registers.V[x] == chip8->registers.V[y])
			{
				chip8->registers.PC += 2;
			}
			break;

			// 6xkk - LD Vx, byte
			// Set Vx = kk.
			// The interpreter puts the value kk into register Vx.
		case 0x6000:
			chip8->registers.V[x] = kk;
			break;

			// 7xkk - ADD Vx, byte
			// Set Vx = Vx + kk.
			// Adds the value kk to the value of register Vx, then stores the result in Vx.
		case 0x7000:
			chip8->registers.V[x] += kk;
			break;

		case 0x8000:

			switch (opcode & 0x000f)
			{
				// 8xy0 - LD Vx, Vy
				// Set Vx = Vy.
				// Stores the value of register Vy in register Vx.
				case 0x0000:
					chip8->registers.V[x] = chip8->registers.V[y];
					break;

					// 8xy1 - OR Vx, Vy
					// Set Vx = Vx OR Vy.
					// Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
				case 0x0001:
					chip8->registers.V[x] |= chip8->registers.V[y];
					break;
					/*
					 * 8xy2 - AND Vx, Vy
					 * Set Vx = Vx AND Vy.
					 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
					 */
				case 0x0002:
					chip8->registers.V[x] &= chip8->registers.V[y];
					break;

					/*
					 * 8xy3 - XOR Vx, Vy
					 * Set Vx = Vx XOR Vy.
					 */
				case 0x0003:
					chip8->registers.V[x] ^= chip8->registers.V[y];
					break;

					/*
					 * 8xy4 - ADD Vx, Vy
					 * Set Vx = Vx + Vy, set VF = carry.
					 * The values of Vx and Vy are added together. If the result is greater than
					 * 8 bits (i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits
					 * of the result are kept, and stored in Vx.
					 */
				case 0x0004:
					const unsigned short sum = chip8->registers.V[x] + chip8->registers.V[y];
					chip8->registers.V[x] = sum;
					if (sum > 255)
					{
						chip8->registers.V[0x0F] = 1;
					}
					else
					{
						chip8->registers.V[0x0F] = 0;
					}
					break;

					/*
					 * 8xy5 - SUB Vx, Vy
					 * Set Vx = Vx - Vy, set VF = NOT borrow.
					 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx,
					 * and the results stored in Vx.
					 */
				case 0x0005:
					if (chip8->registers.V[x] > chip8->registers.V[y])
					{
						chip8->registers.V[0x0F] = 1;
					}
					else
					{
						chip8->registers.V[0x0F] = 0;
					}
					chip8->registers.V[x] -= chip8->registers.V[y];
					break;

					/*
					 * 8xy6 - SHR Vx {, Vy}
					 * Set Vx = Vx SHR 1.
					 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
					 */
				case 0x0006:
					if ((chip8->registers.V[x] & 0b00000001) == 1)
					{
						chip8->registers.V[0x0F] = 1;
					}
					else
					{
						chip8->registers.V[0x0F] = 0;
					}
					chip8->registers.V[x] /= 2;
					break;

					/*
					 * 8xy7 - SUBN Vx, Vy
					 * Set Vx = Vy - Vx, set VF = NOT borrow.
					 * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
					 */
				case 0x0007:
					if (chip8->registers.V[x] < chip8->registers.V[y])
					{
						chip8->registers.V[0x0F] = 1;
					}
					else
					{
						chip8->registers.V[0x0F] = 0;
					}
					chip8->registers.V[x] = chip8->registers.V[y] - chip8->registers.V[x];
					break;

					/*
					 * 8xyE - SHL Vx {, Vy}
					 * Set Vx = Vx SHL 1.
					 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
					 */
				case 0x000E:
					chip8->registers.V[0x0F] = 0;
					if (chip8->registers.V[x] & 0b10000000)
					{
						chip8->registers.V[0x0F] = 1;
					}
					chip8->registers.V[x] *= 2;
					break;
				default:
					puts("opcode not supported");
			}

			break;

			/*
			 * 9xy0 - SNE Vx, Vy
			 * Skip next instruction if Vx != Vy.
			 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
			 */
		case 0x9000:
			if (chip8->registers.V[x] != chip8->registers.V[y])
			{
				chip8->registers.PC += 2;
			}
			break;

			/*
			 * Annn - LD I, addr
			 * Set I = nnn.
			 * The value of register I is set to nnn.
			 */
		case 0xA000:
			chip8->registers.I = nnn;
			break;

			/*
			 * Bnnn - JP V0, addr
			 * Jump to location nnn + V0.
			 * The program counter is set to nnn plus the value of V0.
			 */
		case 0xB000:
			chip8->registers.PC = nnn + chip8->registers.V[0x00];
			break;

			/*
			 * Cxkk - RND Vx, byte
			 * Set Vx = random byte AND kk.
			 */
		case 0xC000:
			srand(clock());
			chip8->registers.V[x] = (rand() % 255) & kk;
			break;

			/*
			 * Dxyn - DRW Vx, Vy, nibble
			 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
			 * The interpreter reads n bytes from memory, starting at the address stored in I.
			 * These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
			 * Sprites are XORed onto the existing screen. If this causes any pixels to be erased,
			 * VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part of it is
			 * outside the coordinates of the display, it wraps around to the opposite side of the screen.
			 * See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more
			 * information on the Chip-8 screen and sprites.
			 */
		case 0xD000:
		{
			const char* sprite = (const char*)&chip8->memory.memory[chip8->registers.I];
			chip8->registers.V[0x0F] = chip8_screen_draw_sprite(&chip8->screen, chip8->registers.V[x], chip8->registers.V[y], sprite, n);
		}
		break;

		case 0xE000:
			switch (opcode & 0x00FF)
			{
				/*
				 * Ex9E - SKP Vx
				 * Skip next instruction if key with the value of Vx is pressed.
				 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
				 */
				case 0x9E:
				{
					const bool is_down = chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]);
					if (is_down)
					{
						chip8->registers.PC += 2;
					}
				}
				break;

				/*
				 * ExA1 - SKNP Vx
				 * Skip next instruction if key with the value of Vx is not pressed.
				 * Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
				 */
				case 0xA1:
				{
					const bool is_down = chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x]);
					if (!is_down)
					{
						chip8->registers.PC += 2;
					}
				}
				break;
				default:
					puts("opcode not supported");
			}
			break;

		case 0xF000:
			switch (opcode & 0x00FF)
			{
				/*
				 * Fx07 - LD Vx, DT
				 * Set Vx = delay timer value.
				 * The value of DT is placed into Vx.
				 */
				case 0x07:
					chip8->registers.V[x] = chip8->registers.delay_timer;
					break;

					/*
					 * Fx0A - LD Vx, K
					 * Wait for a key press, store the value of the key in Vx.
					 * All execution stops until a key is pressed, then the value of that key is stored in Vx.
					 */
				case 0x0A:
					chip8->registers.V[x] = chip8_wait_for_key_press(chip8);
					break;

					/*
					 * Fx15 - LD DT, Vx
					 * Set delay timer = Vx.
					 * DT is set equal to the value of Vx.
					 */
				case 0x15:
					chip8->registers.delay_timer = chip8->registers.V[x];
					break;

					/*
					 * Fx18 - LD ST, Vx
					 * Set sound timer = Vx.
					 * ST is set equal to the value of Vx.
					 */
				case 0x18:
					chip8->registers.sound_timer = chip8->registers.V[x];
					break;

					/*
					 * Fx1E - ADD I, Vx
					 * Set I = I + Vx.
					 * The values of I and Vx are added, and the results are stored in I.
					 */
				case 0x1E:
					chip8->registers.I += chip8->registers.V[x];
					break;

					/*
					 * Fx29 - LD F, Vx
					 * Set I = location of sprite for digit Vx.
					 * The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
					 * See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
					 */
				case 0x29:
					chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
					break;

					/*
					 * Fx33 - LD B, Vx
					 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
					 * The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at
					 * location in I, the tens digit at location I+1, and the ones digit at location I+2.
					 */
				case 0x33:
				{
					const unsigned char hundreds = chip8->registers.V[x] / 100;
					const unsigned char tens = chip8->registers.V[x] / 10 % 10;
					const unsigned char units = chip8->registers.V[x] % 10;
					chip8_memory_set(&chip8->memory, chip8->registers.I, hundreds);
					chip8_memory_set(&chip8->memory, chip8->registers.I + 1, tens);
					chip8_memory_set(&chip8->memory, chip8->registers.I + 2, units);
				}
				break;

				/*
				 * Fx55 - LD [I], Vx
				 * Store registers V0 through Vx in memory starting at location I.
				 * The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
				 */
				case 0x55:
				{
					for (int i = 0; i <= x; ++i)
					{
						chip8_memory_set(&chip8->memory, chip8->registers.I + i, chip8->registers.V[i]);
					}
					break;
				}
				/*
				 * Fx65 - LD Vx, [I]
				 * Read registers V0 through Vx from memory starting at location I.
				 * The interpreter reads values from memory starting at location I into registers V0 through Vx.
				 */
				case 0x65:
				{
					for (int i = 0; i <= x; ++i)
					{
						chip8->registers.V[i] = chip8_memory_get(&chip8->memory, chip8->registers.I + i);
					}
				}
				break;

				default:
					puts("opcode not supported");
			}
			break;

		default:
			puts("opcode not supported");
	}
}

void chip8_exec(struct chip8* chip8, const unsigned short opcode)
{
	switch (opcode)
	{
		// 00E0 - CLS
		// Clear the display.
		case 0x00E0:
			chip8_screen_clear(&chip8->screen);
			break;

			// 00EE - RET
			// Return from a subroutine.
			// The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
		case 0x00EE:
			chip8->registers.PC = chip8_stack_pop(chip8);
			break;

		default:
			chip8_exec_extended(chip8, opcode);
	}
}

void chip8_load(struct chip8* chip8, const char* buf, const size_t size)
{
	assert(size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
	memcpy(&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
	chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}
