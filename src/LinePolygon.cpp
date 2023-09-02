#include "Utils/Constants.hpp"
#include "LinePolygon.hpp"
#include "Game.hpp"

#include <SDL2/SDL.h>

#include <vector>
#include <iostream>
#include <cmath>

LinePolygon::LinePolygon(Game* game) : game_(game), furthest_distance_squared_(std::numeric_limits<double>::min()), removed_(false)
{
	center_.x = 0.0;
	center_.y = 0.0;
	acceleration_vector_.x = 0.0;
	acceleration_vector_.y = 0.0;
	velocity_vector_.x = 0.0;
	velocity_vector_.y = 0.0;
}

void LinePolygon::Render()
{
	SDL_SetRenderDrawColor(game_->Renderer(), 0xFF, 0xFF, 0xFF, 0xFF);        

	for (std::size_t i = 0; i < geometry_.size(); ++i)
	{
		const int current_index = i;
		const int next_index = (i + 1) % geometry_.size();
    	
    	SDL_RenderDrawLineF(game_->Renderer(), geometry_[current_index].x, geometry_[current_index].y, geometry_[next_index].x, geometry_[next_index].y);
	}
}

const std::vector<SDL_FPoint>& LinePolygon::Geometry()
{
	return geometry_;
}

void LinePolygon::AddPoint(double x, double y)
{
	SDL_FPoint point;
	point.x = x;
	point.y = y;

	geometry_.push_back(point);
}

void LinePolygon::TranslateGeometry(double x, double y)
{
	center_.x += x;
	center_.y += y;

	for (SDL_FPoint& point : geometry_)
	{
		point.x += x;
		point.y += y;
	}
}

void LinePolygon::RotateGeometry(int degrees)
{
	if (degrees == 0)
	{
		return;
	}

	for (SDL_FPoint& point : geometry_)
	{
		point = RotatePoint(point, center_, degrees);
	}
}

SDL_FPoint LinePolygon::RotatePoint(const SDL_FPoint& point, const SDL_FPoint& pivot, int degrees)
{
	SDL_FPoint result_point = point;

	const double pi = std::acos(-1);
	const double deg_to_rad = static_cast<double>(degrees) * pi / 180.0;
	const double sin_degrees = std::sin(deg_to_rad);
	const double cos_degrees = std::cos(deg_to_rad);

	const double new_x = (result_point.x - pivot.x) * cos_degrees - (result_point.y - pivot.y) * sin_degrees;
	const double new_y = (result_point.x - pivot.x) * sin_degrees + (result_point.y - pivot.y) * cos_degrees;

	result_point.x = new_x + pivot.x;
	result_point.y = new_y + pivot.y;

	return result_point;
}

void LinePolygon::Scale(double scale_factor)
{
	center_.x *= scale_factor;
	center_.y *= scale_factor;

	for (SDL_FPoint& point : geometry_)
	{
		point.x *= scale_factor;
		point.y *= scale_factor;
	}
}

void LinePolygon::VecSetLength(SDL_FPoint* vector, double length)
{
	const double magnitude = std::sqrt(vector->x * vector->x + vector->y * vector->y);
	
	vector->y = vector->y / magnitude * length;
	vector->x = vector->x / magnitude * length;
}

void LinePolygon::WrapGeometryAroundScreen()
{
	bool out_of_left = std::all_of(geometry_.begin(), geometry_.end(), [](const SDL_FPoint& point)
	{
		return point.x < 0;
	});

	if (out_of_left)
	{
		center_.x += constants::screen_width;

		for (SDL_FPoint& point : geometry_)
		{
			point.x += constants::screen_width;
		}
	}

	bool out_of_right = std::all_of(geometry_.begin(), geometry_.end(), [](const SDL_FPoint& point)
	{
		return point.x > constants::screen_width;
	});

	if (out_of_right)
	{
		center_.x -= constants::screen_width;

		for (SDL_FPoint& point : geometry_)
		{
			point.x -= constants::screen_width;
		}
	}

	bool out_of_top = std::all_of(geometry_.begin(), geometry_.end(), [](const SDL_FPoint& point)
	{
		return point.y < 0;
	});

	if (out_of_top)
	{
		center_.y += constants::screen_height;

		for (SDL_FPoint& point : geometry_)
		{
			point.y += constants::screen_height;
		}
	}

	bool out_of_bottom = std::all_of(geometry_.begin(), geometry_.end(), [](const SDL_FPoint& point)
	{
		return point.y > constants::screen_height;
	});

	if (out_of_bottom)
	{
		center_.y -= constants::screen_height;

		for (SDL_FPoint& point : geometry_)
		{
			point.y -= constants::screen_height;
		}
	}
}