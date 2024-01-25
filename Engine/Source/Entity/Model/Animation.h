#pragma once

#include "Util/Math.h"
#include <vector>

namespace Cosmos::ModelHelper
{
	// forward declaration
	struct Node;

	struct Animation
	{
	public:

		struct Channel
		{
			typedef enum PathType 
			{
				TRANSLATION,
				ROTATION,
				SCALE
			} PathType;

			PathType pathType = TRANSLATION;
			Node* node = nullptr;
			uint32_t samplerIndex = 0;
		};

		struct Sampler
		{
			typedef enum InterpolationType
			{
				LINEAR,
				STEP,
				CUBIC_SPLINE
			} InterpolationType;

			InterpolationType interpolationType = LINEAR;
			std::vector<float> inputs = {};
			std::vector<glm::vec4> outputs = {};
		};

	public:

		// constructor
		Animation() = default;

		// destructor
		~Animation() = default;

	public:

		std::string name = {};
		std::vector<Channel> channels = {};
		std::vector<Sampler> samplers = {};
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};
}