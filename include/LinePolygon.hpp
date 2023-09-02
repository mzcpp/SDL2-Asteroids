#ifndef LINE_POLYGON_HPP
#define LINE_POLYGON_HPP

#include <SDL2/SDL.h>

#include <vector>

class Game;

class LinePolygon
{
protected:
	Game* game_;
	std::vector<SDL_FPoint> geometry_;

public:
	double furthest_distance_squared_;
	bool removed_;

	SDL_FPoint center_;
	SDL_FPoint acceleration_vector_;
	SDL_FPoint velocity_vector_;

	LinePolygon(Game* game);
	
	virtual ~LinePolygon() = default;

	virtual void HandleEvent(SDL_Event* e) = 0;
	
	virtual void Tick() = 0;

	virtual void MoveGeometry(double ax, double ay) = 0; 

	void Render();

	const std::vector<SDL_FPoint>& Geometry();

	void AddPoint(double x, double y);

	void TranslateGeometry(double x, double y);

	void RotateGeometry(int degrees);
	
	SDL_FPoint RotatePoint(const SDL_FPoint& point, const SDL_FPoint& pivot, int degrees);
	
	void Scale(double scale_factor);

	void VecSetLength(SDL_FPoint* vector, double length);

	void WrapGeometryAroundScreen();
};

#endif
