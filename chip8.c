#include <stdint.h>
#include <err.h>
#include <SDL.h>

int
main(int argc, char **argv)
{
	uint16_t opcode, I, pc, sp, delaytimer, soundtimer;
	uint8_t mem[4096];
	uint8_t V[16];
	uint8_t gfx[64 * 32];
	uint16_t stack[16];
	uint8_t key[16];

	SDL_Event ev;
	SDL_Window *win;
	SDL_Renderer *ren;

	SDL_Init(SDL_INIT_VIDEO);
	win = SDL_CreateWindow("CHIP8", 100, 100, 640, 480,
		SDL_WINDOW_SHOWN);
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

	/* Initialize Emulator */

	for (;;) {
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				goto cleanup;

			if (ev.type == SDL_KEYDOWN)
				goto cleanup;
		}

		/* Emulate Cycle */

		/* Draw Cycle */
		SDL_RenderClear(ren);
		SDL_RenderPresent(ren);

		/* Set Keys */
	}

cleanup:
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}
