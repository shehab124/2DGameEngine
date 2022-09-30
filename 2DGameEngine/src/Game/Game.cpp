#include "Game.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <glm/glm.hpp>
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include <fstream>

Game::Game()
{
	isRunning = false;
	registry = std::make_unique<Registry>();
	assetStore = std::make_unique<AssetStore>();
	Logger::Log("Game Constructor called.");
}

Game::~Game()
{
	Logger::Log("Game Destructor called.");
}

/*
initializing the window using sdl
*/
void Game::Initialize()
{
	//init sdl
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) //fails if we are running on a machine that doesn't have a gui
	{
		Logger::Err("Error initializing SDL.");
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

void Game::LoadLevel(int level)
{
	// Add the system that need to be processed in our game
	registry->AddSystem<MovementSystem>();
	registry->AddSystem<RenderSystem>();
	registry->AddSystem<AnimationSystem>();

	// Adding assets 
	assetStore->AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
	assetStore->AddTexture(renderer, "truck-image", "./assets/images/truck-ford-right.png");
	assetStore->AddTexture(renderer, "tilemap-image", "./assets/tilemaps/jungle.png");
	assetStore->AddTexture(renderer, "chopper-image", "./assets/images/chopper.png");
	assetStore->AddTexture(renderer, "radar-image", "./assets/images/radar.png");


	// Load the tilemap
	int tileSize = 32;
	double tileScale = 2.0;
	int mapNumCols = 25;
	int mapNumRows = 20;

	std::fstream mapFile;
	mapFile.open("./assets/tilemaps/jungle.map");

	for (int y = 0; y < mapNumRows; y++)
	{
		for (int x = 0; x < mapNumCols; x++)
		{
			char ch;
			mapFile.get(ch);
			int srcRectY = std::atoi(&ch) * tileSize;
			mapFile.get(ch);
			int srcRectX = std::atoi(&ch) * tileSize;
			mapFile.ignore();

			Entity tile = registry->CreateEntity();
			tile.AddComponent<TransformComponent>(glm::vec2(x * (tileScale * tileSize), y * (tileScale * tileSize)), glm::vec2(tileScale, tileScale), 0.0);
			tile.AddComponent<SpriteComponent>("tilemap-image", tileSize, tileSize, 0,srcRectX, srcRectY);
		}
	}
	mapFile.close();

	// Create an entity
	Entity chopper = registry->CreateEntity();
	chopper.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(1, 1), 0);
	chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0));
	chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 2);
	chopper.AddComponent<AnimationComponent>(2, 15, true);

	Entity radar = registry->CreateEntity();
	radar.AddComponent<TransformComponent>(glm::vec2(windowWidth - 74 , 10.0), glm::vec2(1, 1), 0);
	radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0));
	radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 1);
	radar.AddComponent<AnimationComponent>(8, 5 , true);

	Entity tank = registry->CreateEntity();
	tank.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(1, 1), 0);
	tank.AddComponent<RigidBodyComponent>(glm::vec2(40.0, 0));
	tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 2);

	Entity truck = registry->CreateEntity();
	truck.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(1.0, 1.0), 0.0);
	truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 50));
	truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 2);
}

void Game::Setup() //initialize game objects
{
	LoadLevel(1);
}

void Game::Update() //UPDATE GAME OBJECTS BASED ON INPUT FROM USER
{
	// IF WE ARE TOO FAST, WE WAIT IN THIS LOOP
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
	if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME)
	{
		SDL_Delay(timeToWait);
	}

	// The difference in ticks since the last frame, converted to seconds
	double deltaTime = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0;

	//Store previous frame time
	millisecsPreviousFrame = SDL_GetTicks();

	// Update the registry to process the entities that are waiting to be created/deleted
	registry->Update();

	// Invoke all the systems that need to update
	registry->GetSystem<MovementSystem>().Update(deltaTime);
	registry->GetSystem<AnimationSystem>().Update();
}

void Game::Render() //UPDATE SCREEN
{
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255); //background color and transparency
	SDL_RenderClear(renderer);

	registry->GetSystem<RenderSystem>().Update(renderer,assetStore);

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