#include "SDLWindow.hpp"
#include "Renderer.hpp"

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

	width = 640;
	height = 480;

	window = SDL_CreateWindow
	(
		"CAM-RE",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_RESIZABLE
	);

	if (window == NULL)
	{
		std::string error = SDL_GetError();
		throw std::runtime_error("Unable to create SDL window: " + error);
	}
}

void CAM::Renderer::SDLWindow::HandleEvents
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* /*wp*/,
	size_t thread,
	CAM::Jobs::Job* /*thisJob*/
)
{
	assert (thread == 0);
	static auto a = 0;
	const auto s = 1000000;

	static std::chrono::time_point<std::chrono::high_resolution_clock> last;
	if (a == s)
	{
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> diff = now - last;
		printf("Ev avg %fms (aka %ffps)\n", diff.count() / s, 1000. / (diff.count() / s));
		last = now;
		a = 0;
	}
	else if (a == 0)
	{
		last = std::chrono::high_resolution_clock::now();
	}
	++a;

	SDL_Event event;
	while(SDL_PollEvent(&event) == 1)
	{
		if (event.type == SDL_QUIT)
		{
			shouldContinue = false;
		}
		else if
		(
			event.type == SDL_WINDOWEVENT
			&& event.window.event == SDL_WINDOWEVENT_RESIZED
			&& (width != event.window.data1 || height != event.window.data2)
		)
		{
			width = event.window.data1;
			height = event.window.data2;

			parent->ResizeEvent(width, height);
		}
	}
}

CAM::Renderer::SDLWindow::~SDLWindow()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}
