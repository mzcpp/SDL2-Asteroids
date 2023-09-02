#include "Bullet.hpp"
#include "Game.hpp"
#include "Utils/Constants.hpp"

#include <SDL2/SDL.h>

#include <iostream>

Bullet::Bullet(Game* game, double x, double y, double vx, double vy) : game_(game), removed_(false), lifetime_(30)
{
	constexpr double bullet_size = 4.0;

	geometry_.x = x;
	geometry_.y = y;
	geometry_.w = bullet_size;
	geometry_.h = bullet_size;

	velocity_vector_.x = vx;
	velocity_vector_.y = vy;
}

void Bullet::Tick()
{
	if (--lifetime_ == 0)
	{
		removed_ = true;
	}

	geometry_.x += velocity_vector_.x;
	geometry_.y += velocity_vector_.y;

	if (geometry_.x > constants::screen_width)
	{
		geometry_.x -= constants::screen_width;
	}
	else if (geometry_.x < 0.0)
	{
		geometry_.x += constants::screen_width;
	}

	if (geometry_.y > constants::screen_height)
	{
		geometry_.y -= constants::screen_height;
	}
	else if (geometry_.y < 0.0)
	{
		geometry_.y += constants::screen_height;
	}

	HandleCollision();
}

void Bullet::Render()
{
	SDL_RenderFillRectF(game_->Renderer(), &geometry_);
}

void Bullet::HandleCollision()
{
	if (removed_)
	{
		return;
	}

	for (const std::unique_ptr<Asteroid>& asteroid : game_->Asteroids())
	{
		const double dist_squared = ((geometry_.x - asteroid->center_.x) * (geometry_.x - asteroid->center_.x)) + ((geometry_.y - asteroid->center_.y) * (geometry_.y - asteroid->center_.y));

    	if (dist_squared < asteroid->furthest_distance_squared_)
		{
			game_->PlayAsteroidExplosionSound();
			asteroid->removed_ = true;
			removed_ = true;
			game_->score_ += 10;
			game_->UpdateScoreText();

			if (asteroid->type_ != AsteroidType::SMALL)
		    {
		        asteroid->Split();
		    }

		    return;
		}
	}
}