#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "LinePolygon.hpp"

class Game;

class Player : public LinePolygon
{
public:
	bool moving_;
	int rotating_degrees_;
	SDL_FPoint direction_vector_;
	int lives_;

	Player(Game* game, int lives);

	void CreatePlayerGeometry();

	void HandleEvent(SDL_Event* e) override;
	
	void Tick() override;

	void MoveGeometry(double ax, double ay) override;

	void Shoot();

	void HandleCollision();

	void ResetPlayer();
};

#endif