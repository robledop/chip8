#ifndef CHIP8_SCREEN_H
#define CHIP8_SCREEN_H

#include <stdbool.h>
#include "config.h"

/*
	http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#2.4

	The original implementation of the Chip-8 language used a 64x32-pixel monochrome
	display with this format:

	(0,0)	(63,0)
	(0,31)	(63,31)

	Some other interpreters, most notably the one on the ETI 660, also had 64x48 and
	64x64 modes. To my knowledge, no current interpreter supports these modes. More
	recently, Super Chip-48, an interpreter for the HP48 calculator, added a 128x64-pixel
	mode. This mode is now supported by most of the interpreters on other platforms.

	Chip-8 draws graphics on screen through the use of sprites. A sprite is a group
	of bytes which are a binary representation of the desired picture. Chip-8 sprites
	may be up to 15 bytes, for a possible sprite size of 8x15.

	Programs may also refer to a group of sprites representing the hexadecimal digits
	0 through F. These sprites are 5 bytes long, or 8x5 pixels. The data should be stored
	in the interpreter area of Chip-8 memory (0x000 to 0x1FF)
*/

struct chip8_screen
{
	bool pixels[CHIP8_HEIGHT][CHIP8_WIDTH];
};

void chip8_screen_set(struct chip8_screen* screen, int x, int y);
bool chip8_screen_is_set(const struct chip8_screen* screen, int x, int y);
bool chip8_screen_draw_sprite(struct chip8_screen* screen, int x, int y, const char* sprite, int num);
void chip8_screen_clear(struct chip8_screen* screen);

#endif


