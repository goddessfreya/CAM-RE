#include "VKInstance.hpp"
#include "Renderer.hpp"

CAM::Renderer::VKInstance::VKInstance
(
	Jobs::WorkerPool* wp,
	Jobs::Job* /*thisJob*/,
	Renderer* parent
) : wp(wp), parent(parent)
{
	uint32_t version;
	VKFNCHECKRETURN(CAM::VKFN::vkEnumerateInstanceVersion(&version));
}

CAM::Renderer::VKInstance::~VKInstance()
{
}
