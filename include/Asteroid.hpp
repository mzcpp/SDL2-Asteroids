#ifndef ASTEROID_HPP
#define ASTEROID_HPP

#include "LinePolygon.hpp"

enum class AsteroidType
{
	LARGE, MEDIUM, SMALL
};

class Asteroid : public LinePolygon
{
public:	
	AsteroidType type_;
	
	Asteroid(Game* game, enum AsteroidType type, double x, double y, double vx, double vy);
	
	void HandleEvent(SDL_Event* e) override;
	
	void Tick() override;

	void MoveGeometry(double ax, double ay) override;

	void Split();
};

#endif
