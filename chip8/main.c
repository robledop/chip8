#include <stdio.h>
#include "SDL.h"
#include "chip8.h"
#include <Windows.h>

const char keyboard_map[CHIP_TOTAL_KEYS] = {
	SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7,
	SDLK_8, SDLK_9, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

void handle_key_down(struct chip8 *chip8, const SDL_Event event)
{
	const int key = event.key.keysym.sym;
	const int vkey = chip8_keyboard_map(&chip8->keyboard, key);
	if (vkey != -1)
	{
		printf("Key pressed %i\n", vkey);
		chip8_keyboard_down(&chip8->keyboard, vkey);
	}
}

void handle_key_up(struct chip8 *chip8, const SDL_Event event)
{
	const int key = event.key.keysym.sym;
	const int vkey = chip8_keyboard_map(&chip8->keyboard, key);
	if (vkey != -1)
	{
		chip8_keyboard_up(&chip8->keyboard, vkey);
	}
}

void draw_pixels(const struct chip8 chip8, SDL_Renderer* renderer)
{
	for (int x = 0; x < CHIP8_WIDTH; x++)
	{
		for (int y = 0; y < CHIP8_HEIGHT; y++)
		{
			if (chip8_screen_is_set(&chip8.screen, x, y))
			{
				SDL_Rect r;
				r.x = x * CHIP8_WINDOW_SCALE;
				r.y = y * CHIP8_WINDOW_SCALE;
				r.w = CHIP8_WINDOW_SCALE;
				r.h = CHIP8_WINDOW_SCALE;
				SDL_RenderFillRect(renderer, &r);
			}
		}
	}
}

int load_rom(const char* filename, char** buf, size_t *size)
{
	FILE* file;

	const int err = fopen_s(&file, filename, "rb");

	if (!file)
	{
		puts("Failed to open file");
		return err;
	}

	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	if (*size <= 0)
	{
		puts("Failed to read file");
		return -1;
	}

	fseek(file, 0, SEEK_SET);

	*buf = malloc(*size * sizeof(char));

	if (*buf == 0)
	{
		puts("Failed to allocate memory");
		return -1;
	}

	const unsigned int result = fread(*buf, *size, 1, file);

	fclose(file);

	if (result != 1)
	{
		puts("Failed to read file");
		fclose(file);
		
		return -1;
	}
	return 0;
}

int main(const int argc, const char** argv)
{
	if (argc < 2)
	{
		puts("Please provide a game rom file.");
		return -1;
	}

	const char* filename = argv[1];

	size_t size;
	char* buf = NULL;
	const int result = load_rom(filename, &buf, &size);

	if (result != 0 || buf == NULL)
	{
		return -1;
	}

	struct chip8 chip8;
	chip8_init(&chip8);
	chip8_load(&chip8, buf, size);
	chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow(
		EMULATOR_WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		CHIP8_WIDTH * CHIP8_WINDOW_SCALE,
		CHIP8_HEIGHT * CHIP8_WINDOW_SCALE,
		SDL_WINDOW_SHOWN);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_TEXTUREACCESS_TARGET);

	while (1)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				goto out;

			case SDL_KEYDOWN:
			{
				handle_key_down(&chip8, event);
			}

			break;

			case SDL_KEYUP:
			{
				handle_key_up(&chip8, event);
			}
			break;

			default:;
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

		draw_pixels(chip8, renderer);

		SDL_RenderPresent(renderer);

		if (chip8.registers.delay_timer > 0)
		{
			Sleep(10);
			chip8.registers.delay_timer -= 1;
		}

		if (chip8.registers.sound_timer > 0)
		{
			Beep(400, 10 * chip8.registers.sound_timer);
			chip8.registers.sound_timer = 0;
		}

		const unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC);
		chip8.registers.PC += 2;
		chip8_exec(&chip8, opcode);
	}

out:
	SDL_DestroyWindow(window);
	return 0;
}


