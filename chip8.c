#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <SDL.h>

struct chip8_state {
	uint8_t       memory[4096];
	uint8_t       V[16];
	uint16_t      pc;
	uint16_t      I;
	uint8_t       gfx[64 * 32];
#define CHIP8_SET_GFX(s, x, y, v) ((s)->gfx[((x)&0x3F)*32+((y)&0x1F)]=(v))
	uint16_t      stack[16];
	uint8_t       sp;
	uint8_t       key[16];
	uint16_t      delay;
	uint16_t      sound;

	SDL_Window   *win;
	SDL_Renderer *ren;
};

static void chip8_init(struct chip8_state *, const char *);
static void chip8_draw(struct chip8_state *);
static void chip8_decode(struct chip8_state *, uint16_t);
static void chip8_decode_math(struct chip8_state *, uint8_t, uint8_t, uint8_t);
static void chip8_decode_draw(struct chip8_state *, uint8_t, uint8_t, uint8_t);
static void chip8_decode_keys(struct chip8_state *, uint8_t, uint8_t);
static void chip8_decode_misc(struct chip8_state *, uint8_t, uint8_t);
static void chip8_handle_keys(struct chip8_state *, SDL_Event *);
static void chip8_handle_timer(struct chip8_state *);
static void chip8_wait_for_key(struct chip8_state *, uint8_t);

int
main(int argc, char **argv)
{
	struct chip8_state state;

	SDL_Event ev;

	uint8_t do_timer = 0;

	if (argc != 2)
		errx(1, "usage: %s program", argv[0]);

	chip8_init(&state, argv[1]);

	SDL_Init(SDL_INIT_VIDEO);
	state.win = SDL_CreateWindow("CHIP-8", 100, 100, 640, 320,
		SDL_WINDOW_SHOWN);
	if (state.win == NULL) {
		SDL_Quit();
		errx(1, "SDL_CreateWindow: %s", SDL_GetError());
	}

	state.ren = SDL_CreateRenderer(state.win, -1,
		SDL_RENDERER_ACCELERATED);
	if (state.ren == NULL) {
		SDL_DestroyWindow(state.win);
		SDL_Quit();
		errx(1, "SDL_CreateRenderer: %s", SDL_GetError());
	}

	for (;;) {
		uint16_t opcode;

		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				goto cleanup;

			chip8_handle_keys(&state, &ev);
		}

		if (state.pc >= 4096)
			goto cleanup;

		opcode = (state.memory[state.pc] << 8)
			| state.memory[state.pc + 1];

		printf("%04X: %04X\n", state.pc, opcode);

		state.pc += 2;
		chip8_decode(&state, opcode);
		chip8_draw(&state);

		if ((do_timer = (do_timer + 1) & 1) == 0)
			chip8_handle_timer(&state);
	}

cleanup:
	SDL_DestroyRenderer(state.ren);
	SDL_DestroyWindow(state.win);
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

static void
chip8_init(struct chip8_state *state, const char *path)
{
	FILE *fp;

	memset(state, '\0', sizeof(struct chip8_state));
	memcpy(state->memory, chip8_fontset, sizeof(chip8_fontset));
	state->pc = 0x200;

	fp = fopen(path, "r");
	if (fp == NULL)
		err(1, "fopen");

	fread(&(state->memory[0x200]), sizeof(uint8_t), 3136, fp);
	fclose(fp);
}

static void
chip8_draw(struct chip8_state *state)
{
	int x, y;
	SDL_Rect r;

	r.w = 10;
	r.h = 10;

	SDL_RenderClear(state->ren);

	for (x = 0; x < 64; ++x) {
		for (y = 0; y < 32; ++y) {
			if (state->gfx[(32 * x) + y] == 0)
				continue;

			r.x = x * 10;
			r.y = y * 10;

			SDL_RenderFillRect(state->ren, &r);
		}
	}

	SDL_RenderPresent(state->ren);
}

static void
chip8_decode(struct chip8_state *state, uint16_t opcode)
{
	uint8_t x, y;

	x = (opcode & 0x0F00) >> 8;
	y = (opcode & 0x00F0) >> 4;

	switch (opcode & 0xF000) {
	case 0x0000:
		if (opcode == 0x00EE)
			state->pc = state->stack[--state->sp];
		else if (opcode == 0x00E0)
			memset(state->gfx, '\0', sizeof(uint8_t) * 32 * 64);
		break;
	case 0x1000:
		state->pc = opcode & 0x0FFF;
		break;
	case 0x2000:
		state->stack[state->sp] = state->pc;
		++state->sp;
		state->pc = opcode & 0x0FFF;
		break;
	case 0x3000:
		if (state->V[x] == (opcode & 0x00FF))
			state->pc += 2;
		break;
	case 0x4000:
		if (state->V[x] != (opcode & 0x00FF))
			state->pc += 2;
		break;
	case 0x5000:
		if (state->V[x] == state->V[y])
			state->pc += 2;
		break;
	case 0x6000:
		state->V[x] = opcode & 0x00FF;
		break;
	case 0x7000:
		state->V[x] += opcode & 0x00FF;
		break;
	case 0x8000:
		chip8_decode_math(state, opcode & 0x000F, x, y);
		break;
	case 0x9000:
		if (state->V[x] != state->V[y])
			state->pc += 2;
		break;
	case 0xA000:
		state->I = opcode & 0x0F00;
		break;
	case 0xB000:
		state->pc = state->V[0] + (opcode & 0x0FFF);
		break;
	case 0xC000:
		state->V[x] = rand() & opcode & 0x00FF;
		break;
	case 0xD000:
		chip8_decode_draw(state, x, y, opcode & 0x000F);
		break;
	case 0xE000:
		chip8_decode_keys(state, x, opcode & 0x00FF);
		break;
	case 0xF000:
		chip8_decode_misc(state, x, opcode & 0x00FF);
		break;
	}
}

static void
chip8_decode_math(struct chip8_state *state, uint8_t op, uint8_t x, uint8_t y)
{
	int16_t temp;

	switch (op) {
	case 0x0:
		state->V[x] = state->V[y];
		break;
	case 0x1:
		state->V[x] |= state->V[y];
		break;
	case 0x2:
		state->V[x] &= state->V[y];
		break;
	case 0x3:
		state->V[x] ^= state->V[y];
		break;
	case 0x4:
		temp = state->V[x] + state->V[y];
		if (temp > 255) {
			state->V[0xF] = 1;
			temp -= 0xFF;
		} else {
			state->V[0xF] = 0;
		}
		state->V[x] = temp;
		break;
	case 0x5:
		temp = state->V[x] - state->V[y];
		if (temp < 0) {
			state->V[0xF] = 0;
			temp += 0xFF;
		} else {
			state->V[0xF] = 1;
		}
		state->V[x] = temp;
		break;
	case 0x6:
		state->V[0xF] = state->V[x] & 0x1;
		state->V[x] >>= 1;
		break;
	case 0x7:
		temp = state->V[y] - state->V[x];
		if (temp < 0) {
			state->V[0xF] = 0;
			temp += 0xFF;
		} else {
			state->V[0xF] = 1;
		}
		state->V[x] = temp;
		break;
	case 0xE:
		temp = state->V[x] << 1;
		if (temp > 255) {
			state->V[0xF] = 1;
			temp -= 0xFF;
		} else {
			state->V[0xF] = 0;
		}
		state->V[x] = temp;
		break;
	}
}

static void
chip8_decode_draw(struct chip8_state *state, uint8_t x, uint8_t y, uint8_t h)
{
	uint8_t i, j, spr;

	state->V[0xF] = 0;

	for (j = 0; j < y; ++j) {
		spr = state->memory[state->I + j];

		for (i = 0; i < 8; ++i) {
			if (spr & 0x80) {
				CHIP8_SET_GFX(state,
					state->V[x] + i, state->V[y] + j, 1);
				state->V[0xF] = 1;
			}

			spr <<= 1;
		}
	}
}

static void
chip8_decode_keys(struct chip8_state *state, uint8_t x, uint8_t op)
{
	switch (op) {
	case 0x9E:
		if (state->key[state->V[x]])
			state->pc += 2;
		break;
	case 0xA1:
		if (!state->key[state->V[x]])
			state->pc += 2;
		break;
	}
}

static void
chip8_decode_misc(struct chip8_state *state, uint8_t x, uint8_t op)
{
	uint8_t i, n;

	switch (op) {
	case 0x07:
		state->V[x] = state->delay;
		break;
	case 0x0A:
		chip8_wait_for_key(state, x);
		break;
	case 0x15:
		state->delay = state->V[x];
		break;
	case 0x18:
		state->sound = state->V[x];
		break;
	case 0x1E:
		state->I += state->V[x];
		break;
	case 0x29:
		state->I = state->V[x] * 5;
		break;
	case 0x33:
		n = state->V[x];
		for (i = 3; i > 0; --i) {
			state->memory[state->I + i - 1] = n % 10;
			n /= 10;
		}
		break;
	case 0x55:
		for (i = 0; i < x; ++i)
			state->memory[state->I + i] = state->V[i];
		break;
	case 0x65:
		for (i = 0; i < x; ++i)
			state->V[i] = state->memory[state->I + i];
		break;
	}
}

static void
chip8_handle_keys(struct chip8_state *state, SDL_Event *ev)
{
	uint8_t setval;

	if (ev->type != SDL_KEYDOWN && ev->type != SDL_KEYUP)
		return;

	if (ev->key.state == SDL_PRESSED)
		setval = 1;
	else if (ev->key.state == SDL_RELEASED)
		setval = 0;

	switch (ev->key.keysym.sym) {
	case SDLK_h:
		state->key[0x4] = setval;
		break;
	case SDLK_l:
		state->key[0x6] = setval;
		break;
	case SDLK_j:
		state->key[0x2] = setval;
		break;
	case SDLK_k:
		state->key[0x8] = setval;
		break;
	/* TODO - add more key codes */
	}
}

static void
chip8_handle_timer(struct chip8_state *state)
{
	if (state->delay > 0)
		--state->delay;

	if (state->sound > 0) {
		if (state->sound == 1) {
			/* TODO - beep */
		}

		--state->sound;
	}
}

static void
chip8_wait_for_key(struct chip8_state *state, uint8_t reg)
{
	SDL_Event ev;

	for (;;) {
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				goto shutdown;

			if (ev.type != SDL_KEYDOWN)
				continue;

			switch (ev.key.keysym.sym) {
			case SDLK_h:
				state->V[reg] = 0x4;
				state->key[0x4] = 1;
				break;
			case SDLK_l:
				state->V[reg] = 0x6;
				state->key[0x6] = 1;
				break;
			case SDLK_j:
				state->V[reg] = 0x2;
				state->key[0x2] = 1;
				break;
			case SDLK_k:
				state->V[reg] = 0x8;
				state->key[0x8] = 1;
				break;
			/* TODO - add more key codes */
			default:
				continue;
			}
			return;
		}

		chip8_draw(state);
	}

shutdown:
	SDL_DestroyRenderer(state->ren);
	SDL_DestroyWindow(state->win);
	SDL_Quit();
	exit(0);
}
