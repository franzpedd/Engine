#pragma once

#include "Core/Defines.h"

#include <memory>

namespace Cosmos
{
	class COSMOS_API Renderer
	{
	public:

		// returns a smart-ptr to a new renderer 
		static std::shared_ptr<Renderer> Create();

		// constructor
		Renderer();

		// destructor
		~Renderer();

	private:


	};
}