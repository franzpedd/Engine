#pragma once

#include "Sound/Source.h"
#include <memory>

namespace Cosmos
{
	struct SoundSourceComponent
	{
		std::shared_ptr<sound::Source> source;

		// constructor
		SoundSourceComponent() = default;
	};
}