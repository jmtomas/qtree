#include <SDL2/SDL.h>

#define upper_bound 10
#define lower_bound 2

struct Point {
	int x;
	int y;

	void render(SDL_Renderer *renderer) {
		SDL_SetRenderDrawColor(renderer, 30, 180, 80, 255);
		SDL_Rect rect = { x - 5, y - 5, 5, 5 };
		SDL_RenderFillRect(renderer, &rect);
	}
};

struct Partition {
	int x;
	int y;
	int w;
	int h;
	int element_count;
	Point *points;
	Partition *subpartitions;

	Partition() : x(0), y(0), w(0), h(0), element_count(0) {
		points = new Point[upper_bound];
		subpartitions = nullptr;
	}

	Partition(int x, int y, int w, int h) : x(x), y(y), w(w), h(h), element_count(0) {
		points = new Point[upper_bound];
		subpartitions = nullptr;
	}

	void render(SDL_Renderer *renderer) {
		if (points) {
			SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
			SDL_Rect rect = { x + 2, y + 2, w - 2, h - 2 };
			SDL_RenderDrawRect(renderer, &rect);
			for (int i = 0; i < upper_bound; i++) {
				points[i].render(renderer);
			}
		} else {
			for (int i = 0; i < 4; i++) {
				subpartitions[i].render(renderer);
			}
		}
	}

	void add_to_subpartition(Point p) {
		if (p.x < w / 2) {
			if (p.y < h / 2) {
				subpartitions[0].add(p);
			} else {
				subpartitions[1].add(p);
			}
		} else {
			if (p.y < h / 2) {
				subpartitions[2].add(p);
			} else {
				subpartitions[3].add(p);
			}
		}
	}

	void add(Point p) {
		if (element_count < upper_bound) {
			points[element_count] = p;
		} else {
			if (!subpartitions) {
				subdivide();
			}
			add_to_subpartition(p);
		}
		element_count++;
	}

	void subdivide() {
		subpartitions = new Partition[4];
		subpartitions[0] = Partition(this->x,               this->y,               this->w / 2, this->h / 2);
		subpartitions[1] = Partition(this->x,               this->y + this->h / 2, this->w / 2, this->h / 2);
		subpartitions[2] = Partition(this->x + this->w / 2, this->y + this->h / 2, this->w / 2, this->h / 2);
		subpartitions[3] = Partition(this->x + this->w / 2, this->y,               this->w / 2, this->h / 2);
		for (int i = 0; i < upper_bound; i++) {
			add_to_subpartition(points[i]);
		}
		delete[] points;
		points = nullptr;
	}
};

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	int window_w = 1280;
	int window_h = 720;
	SDL_Window *window = SDL_CreateWindow("qtree", 0, 0, window_w, window_h, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	Partition *part = new Partition(0, 0, window_w, window_h);
	Point pt = Point();

	int quit = 0;
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			if (SDL_GetMouseState(&pt.x, &pt.y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				part->add(pt);
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
