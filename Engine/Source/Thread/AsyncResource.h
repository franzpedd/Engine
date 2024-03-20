#pragma once

#include "Util/Math.h"

namespace Cosmos::thread
{
	typedef enum class Status
	{
		Success = 0,
		NotReady = 1,
		Error = -1
	} Status;

	struct WaveAsync
	{
		struct Arguments
		{
			std::string path;
			bool stereo = false;
			bool looping = false;
			float pitch = 1.0f;
			float volume = 1.0f;
			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 velocity = glm::vec3(0.0f);
			glm::vec3 at = glm::vec3(0.0f);
			glm::vec3 up = glm::vec3(0.0f);
		};
	};

	// uploads the .wav file asynchronously
	Status UploadWaveFile(WaveAsync::Arguments* args, uint32_t* bufferID, uint32_t* sourceID);
}