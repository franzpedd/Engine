#include "epch.h"
#include "AsyncResource.h"

#include <AL/al.h>
#include "Sound/WaveLoader.h"

namespace Cosmos::thread
{
	Status UploadWaveFile(WaveAsync::Arguments* args, uint32_t* bufferID, uint32_t* sourceID)
	{
		ALenum format = AL_NONE;
		std::vector<uint8_t> monoPCMDataBytes = {};
		sound::WaveLoader audioFile = {};

		// loads wave file into memory
		audioFile.Load(args->path);

		// write PCM data to buffer
		audioFile.WritePCMToBuffer(monoPCMDataBytes);

		// check audio type
		if (audioFile.IsStereo()) 
			args->stereo = true;
		
		// detect bit depth
		if (audioFile.GetBitDepth() == 8) 
			format = args->stereo ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		
		else if (audioFile.GetBitDepth() == 16) 
			format = args->stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		
		else
		{
			LOG_TO_TERMINAL(Logger::Error, "The bit depth %s is not supported by OpenAL, convert it to 8 or 16", audioFile.GetBitDepth());
			return Status::Error;
		}

		// generate buffer
		alGenBuffers(1, bufferID);
		alBufferData(*bufferID, format, monoPCMDataBytes.data(), (ALsizei)monoPCMDataBytes.size(), audioFile.GetSampleRate());
		
		// set all initial flags
		alGenSources(1, sourceID);
		alSourcef(*sourceID, AL_PITCH, args->pitch);
		alSourcef(*sourceID, AL_GAIN, args->volume);
		alSourcei(*sourceID, AL_LOOPING, args->looping);
		alSourcei(*sourceID, AL_BUFFER, *bufferID);

		// 3D spatialization is enabled only on mono-audio
		if (audioFile.IsMono())
		{
			alSource3f(*sourceID, AL_POSITION, args->position[0], args->position[1], args->position[2]);
			alSource3f(*sourceID, AL_VELOCITY, args->velocity[0], args->velocity[1], args->velocity[2]);
			
			float orientation[6] = {};
			orientation[0] = args->at.x;
			orientation[1] = args->at.y;
			orientation[2] = args->at.z;
			orientation[3] = args->up.x;
			orientation[4] = args->up.y;
			orientation[5] = args->up.z;
			
			alSourcefv(*sourceID, AL_ORIENTATION, orientation);
		}

		return Status::Success;
	}
}