#pragma once

#include <stb/stb_vorbis.h>

namespace Cosmos::sound
{
	class SourceStream
	{
	public:

		typedef struct Vorbis
		{
			unsigned int ID;
			stb_vorbis* stream;
			stb_vorbis_info info;
			unsigned int buffers[2];
			unsigned int source;
			int format;
			size_t bufferSize;
			size_t totalSamplesLeft;
			bool shouldLoop;
		} Vorbis;

	public:

		// constructor
		SourceStream(const char* path);

		// destructor
		~SourceStream();

	public:

		// opens a new stream
		bool AudioStreamOpen(const char* path);

		// starts the audio streaming
		void AudioStreamInit(Vorbis* stream);

		// ends the audio streaming
		void AudioStreamTerminate(Vorbis* stream);

		// streams the audio
		bool AudioStream(unsigned int buffer);

		// updates the audio stream
		bool AudioStreamUpdate();

	private:

		Vorbis* mVorbisObject;
	};
}