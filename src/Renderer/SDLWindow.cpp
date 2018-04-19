#include "SDLWindow.hpp"

CAM::Renderer::SDLWindow::SDLWindow
(
	Jobs::WorkerPool* wp,
	Jobs::Job* /*thisJob*/,
	Renderer* parent
) : wp(wp), parent(parent)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::string error = SDL_GetError();
		throw std::runtime_error("Unable to init SDL Video: " + error);
    }

	window = SDL_CreateWindow
	(
		"CAM-RE",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		SDL_WINDOW_RESIZABLE
	);
}

CAM::Renderer::SDLWindow::~SDLWindow()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}
