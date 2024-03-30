#include "Utils/Constants.hpp"
#include "Player.hpp"
#include "Game.hpp"

#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <cmath>
#include <algorithm>

Player::Player(Game* game, int lives) : LinePolygon(game), moving_(false), rotating_degrees_(0), lives_(lives)
{
	CreatePlayerGeometry();
}

void Player::CreatePlayerGeometry()
{
	AddPoint(-12.0, 10.0);
	AddPoint(12.0, 10.0);
	AddPoint(0.0, -30.0);

	TranslateGeometry(constants::screen_width / 2.0, constants::screen_height / 2.0);

	direction_vector_.x = geometry_[2].x - center_.x;
	direction_vector_.y = geometry_[2].y - center_.y;	
}

void Player::HandleEvent(SDL_Event* e)
{
	const int rotation_degrees = 5;
	const double acceleration = 0.2;

	if (e->type == SDL_KEYDOWN && e->key.repeat == 0)
	{
		if (e->key.keysym.sym == SDLK_LEFT)
		{
			rotating_degrees_ -= rotation_degrees;
		}
		else if (e->key.keysym.sym == SDLK_RIGHT)
		{
			rotating_degrees_ += rotation_degrees;
		}
		
		if (e->key.keysym.sym == SDLK_UP)
		{
			moving_ = true;
		
			acceleration_vector_ = direction_vector_;
			VecSetLength(&acceleration_vector_, acceleration);
		}
		if (!game_->game_over_ && e->key.keysym.sym == SDLK_SPACE)
		{
			Shoot();
		}
	}
	else if (e->type == SDL_KEYUP && e->key.repeat == 0)
	{
		if (e->key.keysym.sym == SDLK_LEFT)
		{
			rotating_degrees_ += rotation_degrees;
		}
		else if (e->key.keysym.sym == SDLK_RIGHT)
		{
			rotating_degrees_ -= rotation_degrees;
		}

		if (e->key.keysym.sym == SDLK_UP)
		{
			moving_ = false;
			acceleration_vector_.x = -acceleration_vector_.x;
			acceleration_vector_.y = -acceleration_vector_.y;

			if ((acceleration_vector_.x > 0.0 && velocity_vector_.x > 0.0) || (acceleration_vector_.x < 0.0 && velocity_vector_.x < 0.0))
			{
				acceleration_vector_.x *= -1.0f;
			}

			if ((acceleration_vector_.y > 0.0 && velocity_vector_.y > 0.0) || (acceleration_vector_.y < 0.0 && velocity_vector_.y < 0.0))
			{
				acceleration_vector_.y *= -1.0f;
			}
		}
	}
}
	
void Player::Tick()
{
	RotateGeometry(rotating_degrees_);

	direction_vector_.x = geometry_[2].x - center_.x;
	direction_vector_.y = geometry_[2].y - center_.y;

	if (moving_)
	{
		const double acceleration = 0.2;
		acceleration_vector_ = direction_vector_;
		VecSetLength(&acceleration_vector_, acceleration);
	}

 	MoveGeometry(acceleration_vector_.x, acceleration_vector_.y);
 	HandleCollision();
}

void Player::MoveGeometry(double ax, double ay)
{
	velocity_vector_.x += static_cast<float>(ax);
	velocity_vector_.y += static_cast<float>(ay);
	
	if (moving_)
	{
		velocity_vector_.x = std::clamp(velocity_vector_.x, -5.0f, 5.0f);
		velocity_vector_.y = std::clamp(velocity_vector_.y, -5.0f, 5.0f);
	}
	else
	{
		if (std::fabs(velocity_vector_.x) < std::fabs(ax))
		{
			acceleration_vector_.x = 0.0;
		}

		if (std::fabs(velocity_vector_.y) < std::fabs(ay))
		{
			acceleration_vector_.y = 0.0;
		}
	}

	center_.x += velocity_vector_.x;
	center_.y += velocity_vector_.y;

	for (SDL_FPoint& point : geometry_)
	{
		point.x += velocity_vector_.x;
		point.y += velocity_vector_.y;
	}

	WrapGeometryAroundScreen();
}

void Player::Shoot()
{
	game_->AddBullet(geometry_[2].x, geometry_[2].y, direction_vector_.x, direction_vector_.y);
	game_->PlayShootSound();
}

void Player::HandleCollision()
{
	for (const std::unique_ptr<Asteroid>& asteroid : game_->Asteroids())
	{
		for (int i = 0; i < 3; ++i)
		{
			const double dist_squared = ((geometry_[i].x - asteroid->center_.x) * (geometry_[i].x - asteroid->center_.x)) + ((geometry_[i].y - asteroid->center_.y) * (geometry_[i].y - asteroid->center_.y));

			if (dist_squared < asteroid->furthest_distance_squared_)
			{
				game_->PlayAsteroidExplosionSound();
				asteroid->removed_ = true;
				removed_ = true;
				--lives_;
				game_->UpdateLivesText();

				if (asteroid->type_ != AsteroidType::SMALL)
				{
			        	asteroid->Split();
				}

			    return;
			}
		}
	}
}

void Player::ResetPlayer()
{
	moving_ = false;
	velocity_vector_.x = 0.0;
	velocity_vector_.y = 0.0;
	acceleration_vector_.x = 0.0;
	acceleration_vector_.y = 0.0;

	if (lives_ > 0)
	{
		TranslateGeometry((constants::screen_width / 2.0) - center_.x, (constants::screen_height / 2.0) - center_.y);
		removed_ = false;
	}
	else
	{
		game_->game_over_ = true;
	}
}
