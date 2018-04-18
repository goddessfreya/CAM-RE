#include "Renderer.hpp"

CAM::Renderer::Renderer::Renderer(Jobs::WorkerPool* /*wp*/, Jobs::Job* /*thisJob*/)
{}

void CAM::Renderer::Renderer::DoFrame
(
	void* /*userData*/,
	CAM::Jobs::WorkerPool* /*wp*/,
	size_t /*thread*/,
	CAM::Jobs::Job* /*thisJob*/
) {}
bool CAM::Renderer::Renderer::ShouldContinue() { return false; }
