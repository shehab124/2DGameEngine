#include "Game.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>

Game::Game() //scope resolution, who owns this
{
	isRunning = false;
	std::cout << "Game constructor called\n";
}

Game::~Game()
{
	std::cout << "Game Destructor called";
}

/*
initializing the window using sdl
*/
void Game::Initialize()
{
	//init sdl
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) //fails if we are running on a machine that doesn't have a gui
	{
		std::cerr << "Error initializing SDL." << std::endl;
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	windowHeight = 600;//displayMode.h;
	windowWidth = 800;//displayMode.w;

	// CREATE A WINDOW
	window = SDL_CreateWindow("Chehab's engine", // window name
		SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, //position
		windowWidth,windowHeight, //size
		SDL_WINDOW_BORDERLESS //decoration
		);

	if (!window)
	{
		std::cerr << "Error creating SDL window." << std::endl;
		return;
	}

	//we need a renderer to able to render something in the window
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer)
	{
		std::cerr << "Error creating SDL renderer." << std::endl;
		return;
	}

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	isRunning = true;
}


void Game::ProcessInput() //TAKE INPUT FROM USER
{
	SDL_Event sdlEvent; //struct
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
			case SDL_QUIT: //close window
				isRunning = false;
				break;

			case SDL_KEYDOWN:
				if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
					isRunning = false;
				break;
		}
	}


}

glm::vec2 playerPosition;
glm::vec2 playerVelocity;

void Game::Setup() //initialize game objects
{
	playerPosition = glm::vec2(10.0, 20.0);
	playerVelocity = glm::vec2(1.0, 0);
}

void Game::Update() //UPDATE GAME OBJECTS BASED ON INPUT FROM USER
{
	// IF WE ARE TOO FAST, WE WAIT IN THIS LOOP
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
	if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
	{
		SDL_Delay(timeToWait);
	}

	//frame time since init of SDL
	millisecsPreviousFrame = SDL_GetTicks();

	playerPosition.x += playerVelocity.x;
	playerPosition.y += playerVelocity.y;
}

void Game::Render() //UPDATE SCREEN
{
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255); //background color and transparency
	SDL_RenderClear(renderer);

	//TODO: render all game objects

	// load a PNG texture
	SDL_Surface* surface = IMG_Load("./assets/images/tank-tiger-right.png"); //a surface is represented by edges
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);



	// destination of the rectangle in the screen
	SDL_Rect dstRect = { 
		static_cast<int>(playerPosition.x), static_cast<int>(playerPosition.y)
		, 32, 32
	};
	SDL_RenderCopy(renderer, texture, NULL, &dstRect);
	SDL_DestroyTexture(texture);

	SDL_RenderPresent(renderer);
}

void Game::Run()
{
	Setup();
	while (isRunning) //GAME LOOP
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Destroy()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}