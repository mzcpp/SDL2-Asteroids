#include "Asteroid.hpp"
#include "Game.hpp"

#include <random>
#include <iostream>
#include <limits>
#include <algorithm>

Asteroid::Asteroid(Game* game, enum AsteroidType type, double x, double y, double vx, double vy) : LinePolygon(game), type_(type)
{
	velocity_vector_.x = vx;
	velocity_vector_.y = vy;
	
	center_.x = 0.0;
	center_.y = 0.0;

	SDL_FPoint point_on_circle;
	point_on_circle.x = 0.0; 
	point_on_circle.y = -20.0;

	for (int i = 0; i < 8; ++i)
	{
		AddPoint(point_on_circle.x, point_on_circle.y);
		point_on_circle = RotatePoint(point_on_circle, center_, -45);
	}

	switch (type_)
	{
	case AsteroidType::SMALL:
		Scale(1.0);
		break;
	case AsteroidType::MEDIUM:
		Scale(2.0);
		break;
	case AsteroidType::LARGE:
		Scale(4.0);
		break;
	}

	TranslateGeometry(x, y);

	for (const SDL_FPoint& point : geometry_)
	{
		furthest_distance_squared_ = std::max<double>(furthest_distance_squared_, ((point.x - center_.x) * (point.x - center_.x)) + ((point.y - center_.y) * (point.y - center_.y)));
	}
}

void Asteroid::HandleEvent(SDL_Event* e)
{
	(void) e;
}

void Asteroid::Tick()
{
	MoveGeometry(velocity_vector_.x, velocity_vector_.y);
	RotateGeometry(-1.0);
}

void Asteroid::MoveGeometry(double ax, double ay)
{
	center_.x += ax;
	center_.y += ay;

	for (SDL_FPoint& point : geometry_)
	{
		point.x += ax;
		point.y += ay;
	}

	WrapGeometryAroundScreen();
}

void Asteroid::Split()
{
	std::uniform_real_distribution<double> random_vector_x_{ -5.0, 5.0 };
  	std::uniform_real_distribution<double> random_vector_y_{ -5.0, 5.0 };

	game_->AddAsteroid(std::make_unique<Asteroid>(game_, (type_ == AsteroidType::LARGE ? AsteroidType::MEDIUM : AsteroidType::SMALL), center_.x, center_.y, random_vector_x_(game_->mt_), random_vector_y_(game_->mt_)));
	game_->AddAsteroid(std::make_unique<Asteroid>(game_, (type_ == AsteroidType::LARGE ? AsteroidType::MEDIUM : AsteroidType::SMALL), center_.x, center_.y, random_vector_x_(game_->mt_), random_vector_y_(game_->mt_)));
}