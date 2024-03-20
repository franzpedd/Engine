#include "epch.h"
#include "SourceStream.h"

#include <AL/al.h>

namespace Cosmos::sound
{
	SourceStream::SourceStream(const char* path)
	{
		mVorbisObject = new Vorbis();
	}

	SourceStream::~SourceStream()
	{
		delete mVorbisObject;
	}

	bool SourceStream::AudioStreamOpen(const char* path)
	{
		mVorbisObject->stream = stb_vorbis_open_filename(path, NULL, NULL);

		if (!mVorbisObject->stream)
			return false;

		// get file info
		mVorbisObject->info = stb_vorbis_get_info(mVorbisObject->stream);

		if (mVorbisObject->info.channels == 2)
			mVorbisObject->format = AL_FORMAT_STEREO16;

		else
			mVorbisObject->format = AL_FORMAT_MONO16;

		if (!AudioStream(mVorbisObject->buffers[0]))
			return false;

		if (!AudioStream(mVorbisObject->buffers[1]))
			return false;

		alSourceQueueBuffers(mVorbisObject->source, 2, mVorbisObject->buffers);
		alSourcePlay(mVorbisObject->source);

		mVorbisObject->totalSamplesLeft = stb_vorbis_stream_length_in_samples(mVorbisObject->stream) * mVorbisObject->info.channels;
		return true;
	}

	void SourceStream::AudioStreamInit(Vorbis* stream)
	{
		memset(stream, 0, sizeof(Vorbis));
		alGenSources(1, &stream->source);
		alGenBuffers(2, stream->buffers);
		stream->bufferSize = 4096 * 8;
		stream->shouldLoop = true; // we loop by default
	}

	void SourceStream::AudioStreamTerminate(Vorbis* stream)
	{
		alDeleteSources(1, &stream->source);
		alDeleteBuffers(2, stream->buffers);
		stb_vorbis_close(stream->stream);
		memset(stream, 0, sizeof(Vorbis));
	}

	bool SourceStream::AudioStream(unsigned int buffer)
	{
		constexpr uint32_t BUFFER_SIZE = 65536;
		ALshort pcm[BUFFER_SIZE] = {};
		int  size = 0;
		int  result = 0;

		while (size < BUFFER_SIZE)
		{
			if (stb_vorbis_get_samples_short_interleaved(mVorbisObject->stream, mVorbisObject->info.channels, pcm + size, BUFFER_SIZE - size) > 0)
				size += result * mVorbisObject->info.channels;

			else
				break;
		}

		if (size == 0)
			return false;

		alBufferData(buffer, mVorbisObject->format, pcm, size * sizeof(ALshort), mVorbisObject->info.sample_rate);
		mVorbisObject->totalSamplesLeft -= size;

		return true;
	}

	bool SourceStream::AudioStreamUpdate()
	{
		ALint processed = 0;

		alGetSourcei(mVorbisObject->source, AL_BUFFERS_PROCESSED, &processed);

		while (processed--)
		{
			ALuint buffer = 0;

			alSourceUnqueueBuffers(mVorbisObject->source, 1, &buffer);

			if (!AudioStream(buffer))
			{
				bool shouldExit = true;

				if (mVorbisObject->shouldLoop)
				{
					stb_vorbis_seek_start(mVorbisObject->stream);
					mVorbisObject->totalSamplesLeft = stb_vorbis_stream_length_in_samples(mVorbisObject->stream) * mVorbisObject->info.channels;
					shouldExit = !AudioStream(buffer);
				}

				if (shouldExit)
					return false;
			}

			alSourceQueueBuffers(mVorbisObject->source, 1, &buffer);
		}

		return true;
	}
}