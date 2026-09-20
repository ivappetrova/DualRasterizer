//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include "SDL_image.h"
#include "SDL_syswm.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#if defined(_DEBUG)
	#include "LeakDetector.h"
#endif

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	// Leak detection
	#if defined(_DEBUG)
		LeakDetector detector{};
	#endif

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - ***Iva Petrova/2GD11***",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	// Print controls at startup
	std::cout << std::endl << "///// Controls /////" << std::endl;
	std::cout << std::endl << "Camera Movement:" << std::endl;
	std::cout << "  W/Arrow Up    - Move Forward" << std::endl;
	std::cout << "  S/Arrow Down  - Move Backward" << std::endl;
	std::cout << "  A/Arrow Left  - Move Left" << std::endl;
	std::cout << "  D/Arrow Right - Move Right" << std::endl;
	std::cout << "  Q/E           - Move Down/Up" << std::endl;
	std::cout << std::endl <<" Rendering:" << std::endl;
	std::cout << "  F1 - Toggle Render Mode (Hardware/Software)" << std::endl;
	std::cout << "  F2 - Toggle Rotation" << std::endl;
	std::cout << "  F3 - Toggle FireFX Mesh" << std::endl;
	std::cout << "  F4 - Cycle Texture Sampling (Point/Linear/Anisotropic)" << std::endl;
	std::cout << "  F5 - Cycle Shading Mode" << std::endl;
	std::cout << "  F6 - Toggle Normal Map" << std::endl;
	std::cout << "  F7 - Toggle Depth Buffer" << std::endl;
	std::cout << "  F8 - Toggle BoundingBox Visualization (Software Only)" << std::endl;
	std::cout << "  F9 - Cycle Cull Mode (Back/Front/None)" << std::endl;
	std::cout << "  F10 - Toggle Uniform Clear Color" << std::endl;
	std::cout << "  F11 - Toggle FPS Display" << std::endl;
	std::cout << "/////////////////";

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	bool printFPS = true;
	
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)
				{
					pRenderer->ToggleRenderMode();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ToggleRotation();
				}
				else if ((e.key.keysym.scancode == SDL_SCANCODE_F3) && (pRenderer->m_UseHardwareRenderer))
				{
					pRenderer->ToggleFireMesh();
				}
				else if ((e.key.keysym.scancode == SDL_SCANCODE_F4) && (pRenderer->m_UseHardwareRenderer))
				{
					pRenderer->CycleSamplingState();
				}
				else if ((e.key.keysym.scancode == SDL_SCANCODE_F5) && (!pRenderer->m_UseHardwareRenderer))
				{
					pRenderer->CycleShadingMode();
				}
				else if ((e.key.keysym.scancode == SDL_SCANCODE_F6) && (!pRenderer->m_UseHardwareRenderer))
				{
					pRenderer->ToggleNormalMap();
				}
				else if ((e.key.keysym.scancode == SDL_SCANCODE_F7) && (!pRenderer->m_UseHardwareRenderer))
				{
					pRenderer->ToggleDepthBuffer();
				}
				else if ((e.key.keysym.scancode == SDL_SCANCODE_F8) && (!pRenderer->m_UseHardwareRenderer))
				{
					pRenderer->ToggleBoundingBoxVisualization();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					pRenderer->CycleCullMode();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pRenderer->ToggleUniformClearColor();
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					printFPS = !printFPS;
					std::cout << "FPS Display: " << (printFPS ? "ON" : "OFF") << std::endl;
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			if (printFPS)
			{
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}