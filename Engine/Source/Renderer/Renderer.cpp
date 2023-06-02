#include "Renderer.h"

namespace Cosmos
{
	std::shared_ptr<Renderer> Renderer::Create()
	{
		return std::make_shared<Renderer>();
	}

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}
}