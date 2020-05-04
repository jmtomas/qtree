#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL.h>

#define upper_bound 10
#define lower_bound 2
#define window_w 1280
#define window_h 720

struct Point {
	float x;
	float y;

	Point() {}
	Point(float x, float y) : x(x), y(y) {}
};

struct Object {
	Point pos;
	Point vel;

	Object() {}
	Object(Point pos, Point vel) : pos(pos), vel(vel) {}

	void render(SDL_Renderer *renderer) {
		SDL_SetRenderDrawColor(renderer, 30, 180, 80, 255);
		SDL_Rect rect = { (int) pos.x - 5, (int) pos.y - 5, 5, 5 };
		SDL_RenderFillRect(renderer, &rect);
	}
};

struct Partition {
	float x;
	float y;
	float w;
	float h;
	int element_count;
	Object *objects;
	Partition *subpartitions;

	Partition() {}
	Partition(float x, float y, float w, float h) : x(x), y(y), w(w), h(h), element_count(0) {
		objects = new Object[upper_bound];
		subpartitions = nullptr;
	}

	void render(SDL_Renderer *renderer) {
		if (objects) {
			SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
			SDL_Rect rect = { (int) x + 2, (int) y + 2, (int) w - 2, (int) h - 2 };
			SDL_RenderDrawRect(renderer, &rect);
			for (int i = 0; i < upper_bound; i++) {
				objects[i].render(renderer);
			}
		} else {
			for (int i = 0; i < 4; i++) {
				subpartitions[i].render(renderer);
			}
		}
	}

	void add_to_subpartition(Object o) {
		if (o.pos.x < x + w / 2) {
			if (o.pos.y < y + h / 2) {
				subpartitions[0].add(o);
			} else {
				subpartitions[1].add(o);
			}
		} else {
			if (o.pos.y < y + h / 2) {
				subpartitions[2].add(o);
			} else {
				subpartitions[3].add(o);
			}
		}
	}

	void add(Object o) {
		if (element_count < upper_bound) {
			objects[element_count] = o;
		} else {
			if (!subpartitions) {
				subdivide();
			}
			add_to_subpartition(o);
		}
		element_count++;
	}

	void subdivide() {
		subpartitions = new Partition[4];
		float half_w = w / 2;
		float half_h = h / 2;
		subpartitions[0] = Partition(x,          y,           half_w, half_h);
		subpartitions[1] = Partition(x,          y + half_h,  half_w, half_h);
		subpartitions[2] = Partition(x + half_w, y,           half_w, half_h);
		subpartitions[3] = Partition(x + half_w, y + half_h,  half_w, half_h);
		for (int i = 0; i < upper_bound; i++) {
			add_to_subpartition(objects[i]);
		}
		delete[] objects;
		objects = nullptr;
	}
};

float random_normalized_float() {
	return (float) rand() / (float) RAND_MAX;
}

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("qtree", 0, 0, window_w, window_h, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	srand((unsigned int)time(0));

	Partition *part = new Partition(0, 0, window_w, window_h);
	int particle_count = 11;
	if (argc > 1) {
		particle_count = atoi(argv[1]);
	}
	Object *objs = new Object[particle_count];
	for (int i = 0; i < particle_count; i++) {
		Point pos = Point(random_normalized_float() * window_w, random_normalized_float() * window_h);
		Point vel = Point(random_normalized_float() - 0.5, random_normalized_float() - 0.5);
		objs[i] = Object(pos, vel);
		part->add(objs[i]);
		std::cout << i << ": " << objs[i].pos.x << " " << objs[i].pos.y << std::endl;
	}

	int quit = 0;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}
		SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
		SDL_RenderClear(renderer);
		part->render(renderer);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
