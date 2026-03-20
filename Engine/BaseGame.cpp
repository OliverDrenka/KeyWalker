#include "base.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include "BaseGame.h"

BaseGame::BaseGame(const Window& window)
	: m_Window{ window }
	, m_Viewport{ 0,0,window.width,window.height }
	, m_pWindow{ nullptr }
	, m_pContext{ nullptr }
	, m_Initialized{ false }
	, m_MaxElapsedSeconds{ 0.1f }
{
	InitializeGameEngine();
}

BaseGame::~BaseGame()
{
	CleanupGameEngine();
}

void BaseGame::InitializeGameEngine()
{
	// disable console close window button
#ifdef _WIN32
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
#endif

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER/*| SDL_INIT_AUDIO*/) < 0)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling SDL_Init: " << SDL_GetError() << std::endl;
		return;
	}

	// Use OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	// Create window
	m_pWindow = SDL_CreateWindow(
		m_Window.title.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		int(m_Window.width),
		int(m_Window.height),
		SDL_WINDOW_OPENGL);
	if (m_pWindow == nullptr)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling SDL_CreateWindow: " << SDL_GetError() << std::endl;
		return;
	}

	// Create OpenGL context 
	m_pContext = SDL_GL_CreateContext(m_pWindow);
	if (m_pContext == nullptr)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
		return;
	}

	// Set the swap interval for the current OpenGL context,
	// synchronize it with the vertical retrace
	if (m_Window.isVSyncOn)
	{
		if (SDL_GL_SetSwapInterval(1) < 0)
		{
			std::cerr << "BaseGame::Initialize( ), error when calling SDL_GL_SetSwapInterval: " << SDL_GetError() << std::endl;
			return;
		}
	}
	else
	{
		SDL_GL_SetSwapInterval(0);
	}

	// Set the Projection matrix to the identity matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set up a two-dimensional orthographic viewing region.
	glOrtho(0, m_Window.width, 0, m_Window.height, -1, 1); // y from bottom to top

	// Set the viewport to the client window area
	// The viewport is the rectangular region of the window where the image is drawn.
	glViewport(0, 0, int(m_Window.width), int(m_Window.height));

	// Set the Modelview matrix to the identity matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Enable color blending and use alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize PNG loading
	/*
	int imgFlags = IMG_INIT_PNG;
	if ( !( IMG_Init( imgFlags ) & imgFlags ) )
	{
		std::cerr << "BaseGame::Initialize( ), error when calling IMG_Init: " << IMG_GetError( ) << std::endl;
		return;
	}
	*/

	// Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling TTF_Init: " << TTF_GetError() << std::endl;
		return;
	}

	//Initialize SDL_mixer

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		std::cerr << "BaseGame::Initialize( ), error when calling Mix_OpenAudio: " << Mix_GetError() << std::endl;
		return;
	}
	InitControllerMap();

	m_Initialized = true;
}

void BaseGame::Run()
{
	if (!m_Initialized)
	{
		std::cerr << "BaseGame::Run( ), BaseGame not correctly initialized, unable to run the BaseGame\n";
		std::cin.get();
		return;
	}

	// Main loop flag
	bool quit{ false };

	// Set start time
	std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

	//The event loop
	SDL_Event e{};
	while (!quit)
	{
		// Poll next event from queue
		while (SDL_PollEvent(&e) != 0)
		{
			// Handle the polled event
			switch (e.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					this->ProcessKeyDownEvent(e.key);
					break;
				case SDL_KEYUP:
					this->ProcessKeyUpEvent(e.key);
					break;
				case SDL_MOUSEMOTION:
					e.motion.y = int(m_Window.height) - e.motion.y;
					this->ProcessMouseMotionEvent(e.motion);
					break;
				case SDL_MOUSEBUTTONDOWN:
					e.button.y = int(m_Window.height) - e.button.y;
					this->ProcessMouseDownEvent(e.button);
					break;
				case SDL_MOUSEBUTTONUP:
					e.button.y = int(m_Window.height) - e.button.y;
					this->ProcessMouseUpEvent(e.button);
					break;
				case SDL_CONTROLLERDEVICEADDED:
				{
					int joystickIndex = e.cdevice.which;
					if (SDL_IsGameController(joystickIndex)) {
						SDL_GameController* controller = SDL_GameControllerOpen(joystickIndex);
						if (controller) {
							std::cout << "Opened controller: " << SDL_GameControllerName(controller) << std::endl;
						}
						else {
							std::cerr << "Could not open game controller: " << SDL_GetError() << std::endl;
						}
					}
					break;
				}
			
				case SDL_CONTROLLERDEVICEREMOVED:
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					quit = HandleControllerButton(e, SDL_KEYDOWN);
					break;
				case SDL_CONTROLLERBUTTONUP:
					quit = HandleControllerButton(e, SDL_KEYUP);
					break;
				case SDL_CONTROLLERAXISMOTION: 
				{
					SDL_GameController* controller = SDL_GameControllerFromInstanceID(e.caxis.which);
					SDL_GameControllerAxis axis = static_cast<SDL_GameControllerAxis>(e.caxis.axis);
					Sint16 value = SDL_GameControllerGetAxis(controller, axis);
					if (axis == SDL_CONTROLLER_AXIS_LEFTX
						|| axis == SDL_CONTROLLER_AXIS_RIGHTX)
					{					
						SDL_Scancode dirPos = m_ControllerMap[SDL_CONTROLLER_BUTTON_DPAD_LEFT];
						SDL_Scancode dirNeg = m_ControllerMap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT];
						HandleControllerAnalogyJoystick(value, dirPos, dirNeg);
					}
					else if (axis == SDL_CONTROLLER_AXIS_LEFTY
						|| axis == SDL_CONTROLLER_AXIS_RIGHTY)
					{
						SDL_Scancode dirPos = m_ControllerMap[SDL_CONTROLLER_BUTTON_DPAD_UP];
						SDL_Scancode dirNeg = m_ControllerMap[SDL_CONTROLLER_BUTTON_DPAD_DOWN];
						HandleControllerAnalogyJoystick(value, dirPos, dirNeg);
					}
					else if (axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT
						|| axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
					{
						SDL_Scancode key = m_ControllerMap[axis];
						if (value > AXIS_DEADZONE)
						{
							SendKeyEvent(SDL_SCANCODE_0, SDL_KEYDOWN);
						}
						else {
							SendKeyEvent(SDL_SCANCODE_0, SDL_KEYUP);
						}
					}
					break;
				}
			}
		}

		if (!quit)
		{
			// Get current time
			std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();

			// Calculate elapsed time
			float elapsedSeconds = std::chrono::duration<float>(t2 - t1).count();

			// Update current time
			t1 = t2;

			// Prevent jumps in time caused by break points
			elapsedSeconds = std::min(elapsedSeconds, m_MaxElapsedSeconds);

			// Call the BaseGame object 's Update function, using time in seconds (!)
			this->Update(elapsedSeconds);

			// Draw in the back buffer
			this->Draw();

			// Update screen: swap back and front buffer
			SDL_GL_SwapWindow(m_pWindow);
		}
	}
}

void BaseGame::InitControllerMap()
{
	m_ControllerMap = {
		{ SDL_CONTROLLER_BUTTON_A, SDL_SCANCODE_SPACE },
		{ SDL_CONTROLLER_BUTTON_B, SDL_SCANCODE_B },
		{ SDL_CONTROLLER_BUTTON_X, SDL_SCANCODE_X },
		{ SDL_CONTROLLER_BUTTON_Y, SDL_SCANCODE_Y },
		{ SDL_CONTROLLER_BUTTON_BACK, SDL_SCANCODE_I },
		{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_SCANCODE_A},
		{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDL_SCANCODE_D},
		{ SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_SCANCODE_W},
		{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_SCANCODE_S},
		{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_SCANCODE_O},
		{ SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_SCANCODE_K},
		{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, SDL_SCANCODE_P},
		{ SDL_CONTROLLER_BUTTON_RIGHTSTICK, SDL_SCANCODE_L},
	};
}

bool BaseGame::HandleControllerButton(SDL_Event& e, Uint32 type)
{
	Uint8 buttonIdx = e.cbutton.button;
	if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START)
	{
		CleanupGameEngine();
		return true; // quit the game
	}
	else {
		SendKeyEvent(m_ControllerMap[buttonIdx], type);
		return false; // dont' quit
	}

}

void BaseGame::HandleControllerAnalogyJoystick(Sint16 value, SDL_Scancode keyNeg, SDL_Scancode keyPos)
{
	if (abs(value) > AXIS_DEADZONE)
	{
		if (value > 0)
		{
			SendKeyEvent(keyNeg, SDL_KEYUP);
			SendKeyEvent(keyPos, SDL_KEYDOWN);
		}
		else
		{
			SendKeyEvent(keyNeg, SDL_KEYDOWN);
			SendKeyEvent(keyPos, SDL_KEYUP);
		}
	}
	else
	{
		SendKeyEvent(keyNeg, SDL_KEYUP);
		SendKeyEvent(keyPos, SDL_KEYUP);
	}
}

void BaseGame::SendKeyEvent(SDL_Scancode scanCode, Uint32 type)
{
	SDL_Event event;
	SDL_zero(event);
	event.type = type;
	event.key.keysym.scancode = scanCode;
	event.key.keysym.sym = SDL_GetKeyFromScancode(scanCode);
	event.key.state = (type == SDL_KEYDOWN) ? SDL_PRESSED : SDL_RELEASED;
	SDL_PushEvent(&event);
}

void BaseGame::CleanupGameEngine()
{
	SDL_GL_DeleteContext(m_pContext);
	SDL_DestroyWindow(m_pWindow);
	m_pWindow = nullptr;

	//Quit SDL subsystems
	Mix_Quit();
	TTF_Quit();
	SDL_Quit();


	// enable console close window button
#ifdef _WIN32
	HWND hwnd = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_ENABLED);
#endif

}
