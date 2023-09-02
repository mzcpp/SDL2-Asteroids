#include "Game.hpp"
#include "Utils/Constants.hpp"
#include "Asteroid.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <iostream>
#include <cassert>
#include <random>

Game::Game() : 
	title_(constants::game_title), 
	is_running_(false), 
	score_(0), 
	number_of_asteroids_(4), 
	info_toggled_(false), 
	game_over_(false), 
	reset_game_(false), 
	mt_(std::random_device{}()), 
	random_x_(0.0, constants::screen_width), 
	random_y_(0.0, constants::screen_height), 
	score_info_(std::make_unique<Texture>()), 
	lives_info_(std::make_unique<Texture>()), 
	toggle_info_(std::make_unique<Texture>()), 
	info_(std::make_unique<Texture>()), 
	game_over_info_(std::make_unique<Texture>()), 
	player_(std::make_unique<Player>(this, 5)), 
	font_(nullptr), 
	shoot_sfx_(nullptr), 
	asteroid_explosion_sfx_(nullptr), 
	window_(nullptr), 
	renderer_(nullptr)
{
	SpawnAsteroids(number_of_asteroids_++);
}

Game::~Game()
{
	Finalize();
}

bool Game::Initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not be initialized! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
	{
		printf("%s\n", "Warning: Texture filtering is not enabled!");
	}

	window_ = SDL_CreateWindow(title_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, constants::screen_width, constants::screen_height, SDL_WINDOW_SHOWN);

	if (window_ == nullptr)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

	if (renderer_ == nullptr)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);

	constexpr int img_flags = IMG_INIT_PNG;

	if (!(IMG_Init(img_flags) & img_flags))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not be initialized! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not be initialized! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}

	return InitResources();
}

void Game::Finalize()
{
	SDL_DestroyWindow(window_);
	window_ = nullptr;

	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;

	TTF_CloseFont(font_);
	font_ = nullptr;

	Mix_FreeChunk(shoot_sfx_);
	shoot_sfx_ = nullptr;

	Mix_FreeChunk(asteroid_explosion_sfx_);
	asteroid_explosion_sfx_ = nullptr;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	Mix_Quit();
}

bool Game::InitResources()
{
	font_ = TTF_OpenFont("res/font/font.ttf", 28);

	if (font_ == nullptr)
	{
		printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	shoot_sfx_ = Mix_LoadWAV("res/sfx/shoot.wav");
	asteroid_explosion_sfx_ = Mix_LoadWAV("res/sfx/explosion.wav");

	SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };

	const std::string score_text = "Score: " + std::to_string(score_);
	score_info_->LoadFromText(renderer_, font_, score_text.c_str(), text_color);

	const std::string lives_text = "Lives: " + std::to_string(player_->lives_);
	lives_info_->LoadFromText(renderer_, font_, lives_text.c_str(), text_color);

    toggle_info_->LoadFromText(renderer_, font_, "Press 'i' to toggle info", text_color);
    info_->LoadFromText(renderer_, font_, "Arrows - move Space - shoot", text_color, 200);
    game_over_info_->LoadFromText(renderer_, font_, "R.I.P. BOZO.            Press 'r' to restart.", text_color, 300);

	return true;
}

void Game::UpdateScoreText()
{
	SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
	const std::string score_text = "Score: " + std::to_string(score_);

	score_info_->LoadFromText(renderer_, font_, score_text.c_str(), text_color);
}

void Game::UpdateLivesText()
{
	SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
	const std::string lives_text = "Lives: " + std::to_string(player_->lives_);

	lives_info_->LoadFromText(renderer_, font_, lives_text.c_str(), text_color);
}

void Game::Run()
{
	if (!Initialize())
	{
		Finalize();
		return;
	}

	is_running_ = true;

	constexpr long double ms = 1.0 / 60.0;
	std::uint64_t last_time = SDL_GetPerformanceCounter();
	long double delta = 0.0;
	
	double timer = SDL_GetTicks();

	int frames = 0;
	int ticks = 0;

	while (is_running_)
	{
		const std::uint64_t now = SDL_GetPerformanceCounter();
		const long double elapsed = static_cast<long double>(now - last_time) / static_cast<long double>(SDL_GetPerformanceFrequency());

		last_time = now;
		delta += elapsed;

		HandleEvents();

		while (delta >= ms)
		{
			Tick();
			delta -= ms;
			++ticks;
		}

		//printf("%Lf\n", delta / ms);
		Render();
		++frames;

		if (SDL_GetTicks() - timer > 1000)
		{
			timer += 1000;
			//printf("Frames: %d, Ticks: %d\n", frames, ticks);
			frames = 0;
			ticks = 0;
		}
	}
}

void Game::Reset()
{
	reset_game_ = false;
	game_over_ = false;

	score_ = 0;
	UpdateScoreText();

	player_->lives_ = 5;
	UpdateLivesText();

	player_->ResetPlayer();

	number_of_asteroids_ = 4;
	asteroids_.clear();
	SpawnAsteroids(number_of_asteroids_);

	bullets_.clear();
}

void Game::HandleEvents()
{
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0)
	{
		if (e.type == SDL_QUIT)
		{
			is_running_ = false;
		}
		
		if (e.type == SDL_KEYUP)
		{
			if (e.key.keysym.sym == SDLK_i)
			{
				info_toggled_ = !info_toggled_;
			}

			if (game_over_ && e.key.keysym.sym == SDLK_r)
			{
				reset_game_ = true;
			}
		}
		
		player_->HandleEvent(&e);
	}
}

void Game::Tick()
{
	if (reset_game_)
	{
		Reset();
	}
	else if (player_->removed_) 
	{
		player_->ResetPlayer();
	}
	else
	{
		player_->Tick();
	}

	if (asteroids_.empty())
	{
		SpawnAsteroids(number_of_asteroids_++);
	}

	auto asteroid_it = asteroids_.begin();

	while (asteroid_it != asteroids_.end())
	{
		if ((*asteroid_it)->removed_)
		{
			asteroid_it = asteroids_.erase(asteroid_it);
			continue;
		}

		(*asteroid_it)->Tick();
		++asteroid_it;
	}

	auto bullet_it = bullets_.begin();

	while (bullet_it != bullets_.end())
	{
		if ((*bullet_it)->removed_)
		{
			bullet_it = bullets_.erase(bullet_it);
			continue;
		}

		(*bullet_it)->Tick();
		++bullet_it;
	}
}

void Game::Render()
{
	SDL_RenderSetViewport(renderer_, NULL);
	SDL_SetRenderDrawColor(renderer_, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(renderer_);

	score_info_->Render(renderer_, (constants::screen_width / 4) - (score_info_->Width() / 2), 0);
	lives_info_->Render(renderer_, (constants::screen_width * (3.0 / 4.0)) - (lives_info_->Width() / 2), 0);
	toggle_info_->Render(renderer_, (constants::screen_width / 2) - (toggle_info_->Width() / 2), constants::screen_height - toggle_info_->Height());

	if (info_toggled_)
	{
		info_->Render(renderer_, 10, constants::screen_height - info_->Height());
	}

	for (const std::unique_ptr<Asteroid>& asteroid : asteroids_)
	{
		asteroid->Render();
	}

	for (const std::unique_ptr<Bullet>& bullet : bullets_)
	{
		bullet->Render();
	}

	if (!game_over_)
	{
		player_->Render();
	}
	else
	{
		game_over_info_->Render(renderer_, (constants::screen_width / 2) - (game_over_info_->Width() / 2), (constants::screen_height / 2) - (game_over_info_->Height() / 2));
	}

	SDL_RenderPresent(renderer_);
}

SDL_Renderer* Game::Renderer() const
{
	return renderer_;
}

const std::list<std::unique_ptr<Asteroid>>& Game::Asteroids()
{
	return asteroids_;
}

void Game::PlayShootSound() const
{
	Mix_PlayChannel(-1, shoot_sfx_, 0);
}
	
void Game::PlayAsteroidExplosionSound() const
{
	Mix_PlayChannel(-1, asteroid_explosion_sfx_, 0);
}

void Game::AddBullet(double x, double y, double vx, double vy)
{
	bullets_.push_front(std::make_unique<Bullet>(this, x, y, vx, vy));
}

void Game::SpawnAsteroids(int amount)
{
	SDL_FPoint points[4] = { { -1.0, 0.0 }, { -1.0, constants::screen_height }, { 0, -1.0 }, { constants::screen_width, 0.0 } };
	std::uniform_real_distribution<double> random_vector_x_{ -2, 2 };
  	std::uniform_real_distribution<double> random_vector_y_{ -2, 2 };

	 for (int i = 0; i < amount; ++i)
	 {
	 	SDL_FPoint spawn_point = points[i % 4];

 		spawn_point.x = spawn_point.x == -1 ? random_x_(mt_) : spawn_point.x;
 		spawn_point.y = spawn_point.y == -1 ? random_y_(mt_) : spawn_point.y;

	 	asteroids_.push_back(std::make_unique<Asteroid>(this, AsteroidType::LARGE, spawn_point.x, spawn_point.y, random_vector_x_(mt_), random_vector_y_(mt_)));
	 }
}

void Game::AddAsteroid(std::unique_ptr<Asteroid> asteroid)
{
	asteroids_.push_back(std::move(asteroid));
}