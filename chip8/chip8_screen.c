#include "chip8_screen.h"
#include <assert.h>
#include <memory.h>

static void chip8_screen_check_bounds(const int x, const int y)
{
	assert(x >= 0 && CHIP8_WIDTH && y >= 0 && y < CHIP8_HEIGHT);
}

void chip8_screen_set(struct chip8_screen* screen, const int x, const int y)
{
	chip8_screen_check_bounds(x, y);
	screen->pixels[y][x] = true;
}

void chip8_screen_clear(struct chip8_screen* screen)
{
	memset(screen->pixels, 0, sizeof(screen->pixels));
}

bool chip8_screen_is_set(const struct chip8_screen* screen, const int x, const int y)
{
	chip8_screen_check_bounds(x, y);
	return screen->pixels[y][x];
}

bool chip8_screen_draw_sprite(struct chip8_screen* screen, const int x, const int y, const char* sprite, const int num)
{
	bool pixel_collision = false;

	for (int ly = 0; ly < num; ly++)
	{
		const char c = sprite[ly];
		for (int lx = 0; lx < 8; lx++)
		{
			if ((c & (0b10000000 >> lx)) == 0)  // NOLINT(clang-diagnostic-gnu-binary-literal)
			{
				continue;
			}

			if (screen->pixels[(ly + y) % CHIP8_HEIGHT][(lx + x) % CHIP8_WIDTH])
			{
				pixel_collision = true;
			}

			// set the pixel taking the wrapping of the screen into consideration using the % operator
			screen->pixels[(ly + y) % CHIP8_HEIGHT][(lx + x) % CHIP8_WIDTH] ^= true;
		}
	}

	return pixel_collision;
}
