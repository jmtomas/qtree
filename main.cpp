#include <cstdlib>
#include <ctime>
#include <random>
#include <functional>
#include <SDL2/SDL.h>

#define upper_bound 10
#define lower_bound 5
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
	bool dirty;

	Object() {}
	Object(Point pos, Point vel) : pos(pos), vel(vel), dirty(false) {}

	void render(SDL_Renderer *renderer) {
		SDL_SetRenderDrawColor(renderer, 30, 150, 80, 255);
		SDL_Rect rect = { (int) pos.x - 5, (int) pos.y - 5, 5, 5 };
		SDL_RenderFillRect(renderer, &rect);
	}

	void update() {
		pos.x += vel.x;
		if (pos.x >= window_w) {
			pos.x -= window_w;
		} else if (pos.x < 0) {
			pos.x += window_w;
		}
		pos.y += vel.y;
		if (pos.y >= window_h) {
			pos.y -= window_h;
		} else if (pos.y < 0) {
			pos.y += window_h;
		}
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
	Partition *north;
	Partition *south;
	Partition *east;
	Partition *west;

	Partition() {}
	Partition(float x, float y, float w, float h) : x(x), y(y), w(w), h(h), element_count(0) {
		objects = new Object[upper_bound];
		subpartitions = nullptr;
		north = nullptr;
		south = nullptr;
		east = nullptr;
		west = nullptr;
	}

	void free() {
		if (objects) delete[] objects;
		if (subpartitions) {
			for (int i = 0; i < 4; i++) {
				subpartitions[i].free();
			}
			delete[] subpartitions;
		}
	}

	void update() {
		if (objects) {
			for (int i = 0; i < element_count; i++) {
				objects[i].update();
				if (objects[i].pos.x < x) {
					west->add(objects[i]);
					objects[i].dirty = true;
				} else if (objects[i].pos.x > x + w) {
					east->add(objects[i]);
					objects[i].dirty = true;
				} else if (objects[i].pos.y < y) {
					north->add(objects[i]);
					objects[i].dirty = true;
				} else if (objects[i].pos.y > y + h) {
					south->add(objects[i]);
					objects[i].dirty = true;
				}
			}
			object_cleanup();
		} else {
			for (int i = 0; i < 4; i++) {
				subpartitions[i].update();
			}
			element_count = 0;
			for (int i = 0; i < 4; i++) {
				element_count += subpartitions[i].element_count;
			}
			if (element_count <= lower_bound) {
				merge();
			}
		}
	}

	void object_cleanup() {
		for (int i = 0; i < element_count; i++) {
			while (objects[i].dirty && element_count > i) {
				element_count--;
				objects[i] = objects[element_count];
			}
		}
	}

	void merge() {
		objects = new Object[upper_bound];
		element_count = 0;
		for (int i = 0; i < 4; i++) {
			if (subpartitions[i].subpartitions) {
				subpartitions[i].merge();
			}
			for (int j = 0; j < subpartitions[i].element_count; j++) {
				add(subpartitions[i].objects[j]);
			}
			delete[] subpartitions[i].objects;
		}
		delete[] subpartitions;
		subpartitions = nullptr;
	}

	void render(SDL_Renderer *renderer) {
		if (objects) {
			SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
			SDL_Rect rect = { (int) x + 2, (int) y + 2, (int) w - 2, (int) h - 2 };
			SDL_RenderDrawRect(renderer, &rect);
			for (int i = 0; i < element_count; i++) {
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
			if (objects) {
				objects[element_count] = o;
			} else {
				add_to_subpartition(o);
			}
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
		subpartitions[0] = Partition(x,          y,          half_w, half_h);
		subpartitions[1] = Partition(x,          y + half_h, half_w, half_h);
		subpartitions[2] = Partition(x + half_w, y,          half_w, half_h);
		subpartitions[3] = Partition(x + half_w, y + half_h, half_w, half_h);

		subpartitions[0].north = north ? north : &subpartitions[1];
		subpartitions[0].south = &subpartitions[1];
		subpartitions[0].east  = &subpartitions[2];
		subpartitions[0].west  = west ? west : &subpartitions[2];

		subpartitions[1].north = &subpartitions[0];
		subpartitions[1].south = south ? south : &subpartitions[0];
		subpartitions[1].east  = &subpartitions[3];
		subpartitions[1].west  = west ? west : &subpartitions[3];

		subpartitions[2].north = north ? north : &subpartitions[3];
		subpartitions[2].south = &subpartitions[3];
		subpartitions[2].east  = east ? east : &subpartitions[0];
		subpartitions[2].west  = &subpartitions[0];

		subpartitions[3].north = &subpartitions[2];
		subpartitions[3].south = south ? south : &subpartitions[2];
		subpartitions[3].east  = east ? east : &subpartitions[1];
		subpartitions[3].west  = &subpartitions[1];

		for (int i = 0; i < upper_bound; i++) {
			add_to_subpartition(objects[i]);
		}
		delete[] objects;
		objects = nullptr;
	}
};

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow("qtree", 0, 0, window_w, window_h, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	std::default_random_engine generator;
	std::normal_distribution<float> pos_dist(0.5, 0.05);
	std::normal_distribution<float> vel_dist(0.0, 0.3);
	auto rand_pos = std::bind(pos_dist, generator);
	auto rand_vel = std::bind(vel_dist, generator);

	Partition *part = new Partition(0, 0, window_w, window_h);
	int particle_count = 1000;
	if (argc > 1) {
		particle_count = atoi(argv[1]);
	}
	for (int i = 0; i < particle_count; i++) {
		Point pos = Point(rand_pos() * window_w, rand_pos() * window_h);
		Point vel = Point(rand_vel(),    rand_vel());
		if (pos.x > 0 && pos.x < window_w && pos.y > 0 && pos.y < window_h) {
			part->add(Object(pos, vel));
		}
	}

	int quit = 0;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
		}
		part->update();
		SDL_SetRenderDrawColor(renderer, 35, 35, 40, 255);
		SDL_RenderClear(renderer);
		part->render(renderer);
		SDL_RenderPresent(renderer);
		SDL_Delay(16);
	}
	part->free();
	delete part;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
