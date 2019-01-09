#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <err.h>
#include <SDL.h>

struct chip8_state {
	uint8_t  memory[4096];
	uint8_t  V[16];
	uint16_t pc;
	uint16_t I;
	uint8_t  gfx[64 * 32];
	uint16_t stack[16];
	uint8_t  key[16];
	uint16_t delay;
	uint16_t sound;
};

static void chip8_init(struct chip8_state *, const char *);

int
main(int argc, char **argv)
{
	uint16_t opcode;
	struct chip8_state state;

	SDL_Event ev;
	SDL_Window *win;
	SDL_Renderer *ren;

	if (argc != 2)
		errx(1, "usage: %s program", argv[0]);

	chip8_init(&state, argv[1]);

	SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("CHIP8", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (win == NULL) {
		SDL_Quit();
		errx(1, "SDL_CreateWindow: %s", SDL_GetError());
	}
	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (ren == NULL) {
		SDL_DestroyWindow(win);
		SDL_Quit();
		errx(1, "SDL_CreateRenderer: %s", SDL_GetError());
	}

	for (;;) {
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				goto cleanup;

			/* Set Keys */
		}

		/* Emulate Cycle */

		/* Draw Cycle */
		SDL_RenderClear(ren);
		SDL_RenderPresent(ren);
	}

cleanup:
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

const static uint8_t chip8_fontset[80] = {
	0xF0,	0x90,	0x90,	0x90,	0xF0,
	0x20,	0x60,	0x20,	0x20,	0x70,
	0xF0,	0x10,	0xF0,	0x80,	0xF0,
	0xF0,	0x10,	0xF0,	0x10,	0xF0,
	0x90,	0x90,	0xF0,	0x10,	0x10,
	0xF0,	0x80,	0xF0,	0x10,	0xF0,
	0xF0,	0x80,	0xF0,	0x90,	0xF0,
	0xF0,	0x10,	0x20,	0x40,	0x40,
	0xF0,	0x90,	0xF0,	0x90,	0xF0,
	0xF0,	0x90,	0xF0,	0x10,	0xF0,
	0xF0,	0x90,	0xF0,	0x90,	0x90,
	0xE0,	0x90,	0xE0,	0x90,	0xE0,
	0xF0,	0x80,	0x80,	0x80,	0xF0,
	0xE0,	0x90,	0x90,	0x90,	0xE0,
	0xF0,	0x80,	0xF0,	0x80,	0xF0,
	0xF0,	0x80,	0xF0,	0x80,	0x80
};

void
chip8_init(struct chip8_state *state, const char *path)
{
	FILE *fp;

	memset(state, '\0', sizeof(struct chip8_state));
	memcpy(&(state->memory[0x50]), chip8_fontset, sizeof(chip8_fontset));
	state->pc = 0x200;

	fp = fopen(path, "r");
	if (fp == NULL)
		err(1, "fopen");

	fread(&(state->memory[0x200]), sizeof(uint8_t), 3136, fp);
	fclose(fp);
}
