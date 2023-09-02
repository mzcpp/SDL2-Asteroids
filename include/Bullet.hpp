#ifndef BULLET_HPP
#define BULLET_HPP

#include <SDL2/SDL.h>

class Game;

class Bullet
{
private:
	Game* game_;

public:
	bool removed_;
	int lifetime_;

	SDL_FRect geometry_;
	SDL_FPoint velocity_vector_;

	Bullet(Game* game, double x, double y, double vx, double vy);

	void Tick();

	void Render();

	void HandleCollision();
};

#endif
