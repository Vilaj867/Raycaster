#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "SDL2_gfxPrimitives.h"
#define SW 1024
#define SH 512
#define HALF_SW SW/2
#define HALF_SH SH/2

#define PI 3.14159265359

#define PLAYER_MOVE_SPEED 10
#define PLAYER_ROTATE_SPEED 0.1f
#define MAX_DEPTH 14
#define FOV PI/3 
#define HALF_FOV FOV/2 

#define CELL_SIZE 64
#define MAP_SIZE_Y 16
#define MAP_SIZE_X 16
#define NUM_RAYS (int)(SW/4)
#define ANGLE_INCREMENT FOV/NUM_RAYS


typedef struct {
	float x, y;
	float dx, dy;
	float angle;
	int mapx, mapy;
}Player;

typedef struct {
	float dx, dy;
	float angle;
}Ray;

int map[MAP_SIZE_Y][MAP_SIZE_X] =
{
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},

};

//converts degrees to rad
float degToRad(float deg) {
	return (deg * PI / 180);
}

void drawRays(SDL_Renderer* renderer, Player* man) {

	int angle_offset = FOV; // rotates initial ray to the left in degrees

	Ray r;
	r.angle = man->angle - HALF_FOV + 0.001;

	//starting (x, y) position and map position
	float ox = man->x;
	float oy = man->y;
	int x_map = man->mapx;
	int y_map = man->mapy;


	float x_vert, y_vert, x_hor, y_hor;
	float depth_vert, depth_hor, delta_depth, final_depth;


	for (int i = 0; i < NUM_RAYS; i++) {

		int dof;

		float cos_a = cos(r.angle);
		float sin_a = sin(r.angle);

		// Horizontal line checking
		if (sin_a > 0) { // looking down
			y_hor = y_map * 64 + 64;
			r.dy = 64;
		}
		else { // looking up
			y_hor = y_map * 64 - 0.0001;
			r.dy = -64;
		}

		depth_hor = (y_hor - oy) / sin_a;
		x_hor = ox + depth_hor * cos_a;

		// Static added length of ray
		delta_depth = r.dy / sin_a;
		r.dx = delta_depth * cos_a;

		//DDA algorithm application
		dof = 0;
		while (dof < MAX_DEPTH) {

			int x_map_check = (int)x_hor / 64;
			int y_map_check = (int)y_hor / 64;
			if (y_map_check < 0) {
				y_map_check *= -1;
			}

			if (x_map_check < 0) {
				x_map_check *= -1;
			}

			if (x_map_check < MAP_SIZE_X &&
				y_map_check < MAP_SIZE_Y &&
				map[y_map_check][x_map_check] == 1) {
				dof = MAX_DEPTH;
			}
			else {
				x_hor += r.dx;
				y_hor += r.dy;
				depth_hor += delta_depth;
				dof++;
			}

		}

		//-----------------------------------------------------//

		// Vertical line checking
		if (cos_a > 0) {  // looking right
			x_vert = x_map * 64 + 64;
			r.dx = 64;
		}
		else { // looking left
			x_vert = x_map * 64 - 0.0001;
			r.dx = -64;
		}

		depth_vert = (x_vert - ox) / cos_a;
		y_vert = oy + depth_vert * sin_a;

		// Static added length of ray
		delta_depth = r.dx / cos_a;
		r.dy = delta_depth * sin_a;

		//DDA algorithm application
		dof = 0;
		while (dof < MAX_DEPTH) {

			int x_map_check = (int)x_vert / 64;
			int y_map_check = (int)y_vert / 64;

			if (y_map_check < 0) {
				y_map_check *= -1;
			}

			if (x_map_check < 0) {
				x_map_check *= -1;
			}

			if (x_map_check < MAP_SIZE_X &&
				y_map_check < MAP_SIZE_Y &&
				map[y_map_check][x_map_check] == 1) {
				dof = MAX_DEPTH;
			}
			else {
				x_vert += r.dx;
				y_vert += r.dy;
				depth_vert += delta_depth;
				dof++;
			}

		}

		//-----------------------------------------------------//

		if (depth_vert > depth_hor) {
			final_depth = depth_hor;
			SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
			//thickLineRGBA(renderer, ox, oy, ox + final_depth * cos_a, oy + final_depth * sin_a, 1, 255, 0, 0, 255);
		}
		else {
			final_depth = depth_vert;
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
			//thickLineRGBA(renderer, ox, oy, ox + final_depth * cos_a, oy + final_depth * sin_a, 1, 255, 0, 0, 255);
		}

		//Draw 3D walls
		float proj_dist = CELL_SIZE / tan(HALF_FOV);
		float proj_height = proj_dist*500 / (final_depth*cos(man->angle - r.angle) + 0.001);

		SDL_Rect rect = { i * 4, HALF_SH - proj_height / 2, 4, proj_height };
		//SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &rect);

		// increment for next ray
		r.angle += ANGLE_INCREMENT;

	}

}

void drawStuff(SDL_Renderer* renderer, Player* man) {

	// clear background to black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	drawRays(renderer, man);

}

bool processEvents(SDL_Window* window, Player* man) {

	SDL_Event e;
	bool loopOn = true;

	while (SDL_PollEvent(&e)) {

		switch (e.type) {
		case(SDL_KEYDOWN):
			switch (e.key.keysym.sym) {
			case(SDLK_ESCAPE):
				printf("Escape was pressed\n\n");
				loopOn = false;
				if (window) {
					SDL_DestroyWindow(window);
					loopOn = false;
				}
				break;
			}
		}
	}

	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_LEFT]) { // left
		man->angle -= PLAYER_ROTATE_SPEED;
		if (man->angle < 0) {
			man->angle += 2 * PI;
		}
		man->dx = cos(man->angle) * PLAYER_MOVE_SPEED;
		man->dy = sin(man->angle) * PLAYER_MOVE_SPEED;

	}

	if (state[SDL_SCANCODE_RIGHT]) { // right
		man->angle += PLAYER_ROTATE_SPEED;
		if (man->angle > 2 * PI) {
			man->angle -= 2 * PI;
		}
		man->dx = cos(man->angle) * PLAYER_MOVE_SPEED;
		man->dy = sin(man->angle) * PLAYER_MOVE_SPEED;
	}

	if (state[SDL_SCANCODE_UP]) { // up
		int nextMapX = floor((man->x + man->dx) / 64);
		int nextMapY = floor((man->y + man->dy) / 64);
		man->mapx = floor((man->x) / 64);
		man->mapy = floor((man->y) / 64);
		float mag = sqrt(pow(man->dx, 2) + pow(man->dy, 2));

		if (map[nextMapY][man->mapx] == 1) {
			man->x += man->dx;
		}
		else if (map[man->mapy][nextMapX] == 1) {
			man->y += man->dy;
		}
		else {
			man->x += man->dx;
			man->y += man->dy;
		}
	}

	if (state[SDL_SCANCODE_DOWN]) { // down
		int nextMapX = floor((man->x - man->dx) / 64);
		int nextMapY = floor((man->y - man->dy) / 64);
		man->mapx = floor((man->x) / 64);
		man->mapy = floor((man->y) / 64);
		float mag = sqrt(pow(man->dx, 2) + pow(man->dy, 2));

		if (map[nextMapY][man->mapx] == 1) {
			man->x -= man->dx;
		}
		else if (map[man->mapy][nextMapX] == 1) {
			man->y -= man->dy;
		}
		else {
			man->x -= man->dx;
			man->y -= man->dy;
		}
	}

	return loopOn;
}

int main(int argc, char* argv[]) {

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window;
	SDL_Renderer* renderer;
	bool loopOn = true;

	//initialize player starting position
	Player man;
	man.x = SW / 4;
	man.y = SH / 2;
	man.angle = 0;
	man.dx = cos(man.angle) * PLAYER_MOVE_SPEED;
	man.dy = sin(man.angle) * PLAYER_MOVE_SPEED;
	man.mapx = floor((man.x) / 64);
	man.mapy = floor((man.y) / 64);

	window = SDL_CreateWindow("cool thing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SW, SH, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//game loop
	while (loopOn) {

		if (!processEvents(window, &man)) {
			loopOn = false;
		}

		drawStuff(renderer, &man);
		SDL_RenderPresent(renderer);

		SDL_Delay(30);

	}

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

	return 0;

}