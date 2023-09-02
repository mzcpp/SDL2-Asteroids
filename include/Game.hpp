#ifndef GAME_HPP
#define GAME_HPP

#include "Bullet.hpp"
#include "Texture.hpp"
#include "Player.hpp"
#include "Asteroid.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <memory>
#include <random>
#include <list>

class Game
{
public:
	const char* title_;
	bool is_running_;
	int score_;
	int number_of_asteroids_;
	bool info_toggled_;
	bool game_over_;
	bool reset_game_;

	std::mt19937_64 mt_;
	std::uniform_int_distribution<double> random_x_;
  	std::uniform_int_distribution<double> random_y_;

private:
	std::unique_ptr<Texture> score_info_;
	std::unique_ptr<Texture> lives_info_;
	std::unique_ptr<Texture> toggle_info_;
	std::unique_ptr<Texture> info_;
	std::unique_ptr<Texture> game_over_info_;

	std::unique_ptr<Player> player_;
	std::list<std::unique_ptr<Asteroid>> asteroids_;
	std::list<std::unique_ptr<Bullet>> bullets_;

	TTF_Font* font_;
	Mix_Chunk* shoot_sfx_;
	Mix_Chunk* asteroid_explosion_sfx_;

	SDL_Window* window_;
	SDL_Renderer* renderer_;

public:
	Game();
	
	~Game();

	bool Initialize();
	
	void Finalize();

	bool InitResources();

	void UpdateScoreText();

	void UpdateLivesText();
	
	void Run();

	void Reset();

	void HandleEvents();

	void Tick();

	void Render();

	SDL_Renderer* Renderer() const;

	const std::list<std::unique_ptr<Asteroid>>& Asteroids();

	void PlayShootSound() const;
	
	void PlayAsteroidExplosionSound() const;
	
	void AddBullet(double x, double y, double vx, double vy);

	void SpawnAsteroids(int amount);
	
	void AddAsteroid(std::unique_ptr<Asteroid> asteroid);
};

#endif
