#include "epch.h"
#include "Source.h"

#include "WaveLoader.h"
#include <AL/al.h>

namespace Cosmos::sound
{
	Source::Source(const char* path)
		: mPath(path)
	{
		// load wave file
		WaveLoader audioFile;
		audioFile.Load(path);
		std::vector<uint8_t> monoPCMDataBytes;
		audioFile.WritePCMToBuffer(monoPCMDataBytes);

		ALenum format = AL_NONE;

		if (audioFile.GetBitDepth() == 8)
		{
			format = audioFile.IsStereo() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		}

		else if (audioFile.GetBitDepth() == 16)
		{
			format = audioFile.IsStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		}

		else
		{
			LOG_TO_TERMINAL(Logger::Error, "The bit depth %s is not supported by OpenAL, convert it to 8 or 16", audioFile.GetBitDepth());
			return;
		}

		alGenBuffers(1, &mBufferID);
		alBufferData(mBufferID, format, monoPCMDataBytes.data(), (ALsizei)monoPCMDataBytes.size(), audioFile.GetSampleRate());

		// set all initial flags
		alGenSources(1, &mSourceID);
		alSourcef(mSourceID, AL_PITCH, mPitch);
		alSourcef(mSourceID, AL_GAIN, mVolume);
		alSourcei(mSourceID, AL_LOOPING, mLoop);
		alSourcei(mSourceID, AL_BUFFER, mBufferID);

		// 3D stuff is enabled only on mono-audio
		if (audioFile.IsMono())
		{
			alSource3f(mSourceID, AL_POSITION, mPosition[0], mPosition[1], mPosition[2]);
			alSource3f(mSourceID, AL_VELOCITY, mVelocity[0], mVelocity[1], mVelocity[2]);
			alSourcefv(mSourceID, AL_ORIENTATION, mOrientation);

			mStereo = false;
		}
	}

	Source::~Source()
	{
		if (mStatus != Stopped)
			Stop();

		alDeleteSources(1, &mSourceID);

		if (mBufferID && alIsBuffer(mBufferID))
		{
			alDeleteBuffers(1, &mBufferID);
		}
	}

	unsigned int Source::GetBufferID() const
	{
		return mBufferID;
	}

	unsigned int Source::GetSourceID() const
	{
		return mSourceID;
	}

	const char* Source::GetPath() const
	{
		return mPath;
	}

	const Source::Status Source::GetStatus() const
	{
		return mStatus;
	}

	void Source::Start()
	{
		alSourcePlay(mSourceID);
		mStatus = Playing;
	}

	void Source::Stop()
	{
		alSourceStop(mSourceID);
		mStatus = Stopped;
	}

	void Source::Pause()
	{
		alSourcePause(mSourceID);
		mStatus = Paused;
	}

	void Source::Resume()
	{
		alSourcePlay(mSourceID);
		mStatus = Playing;
	}

	void Source::Rewind(bool restart)
	{
		alSourceRewind(mSourceID);
		mStatus = Initial;

		if (restart)
		{
			Start();
		}
	}

	void Source::GetPosition(float* x, float* y, float* z) const
	{
		*x = mPosition[0];
		*y = mPosition[1];
		*z = mPosition[2];
	}

	void Source::SetPosition(const float& x, const float& y, const float& z)
	{
		if (mStereo)
		{
			LOG_TO_TERMINAL(Logger::Warn, "Stereo audios Spatialization is not supported by OpenAL");
			return;
		}

		alSource3f(mSourceID, AL_POSITION, x, y, z);
		mPosition[0] = x;
		mPosition[1] = y;
		mPosition[2] = z;
	}

	void Source::GetVelocity(float* x, float* y, float* z) const
	{
		*x = mVelocity[0];
		*y = mVelocity[1];
		*z = mVelocity[2];
	}

	void Source::SetVelocity(const float& x, const float& y, const float& z)
	{
		if (mStereo)
		{
			LOG_TO_TERMINAL(Logger::Warn, "Stereo audios Spatialization is not supported by OpenAL");
			return;
		}

		alSource3f(mSourceID, AL_VELOCITY, x, y, z);
		mVelocity[0] = x;
		mVelocity[1] = y;
		mVelocity[2] = z;
	}

	float Source::GetVolume() const
	{
		return mVolume;
	}

	void Source::SetVolume(const float& volume)
	{
		alSourcef(mSourceID, AL_GAIN, volume);
		mVolume = volume;
	}

	bool Source::GetLooping() const
	{
		return mLoop;
	}

	void Source::SetLooping(bool loop)
	{
		alSourcei(mSourceID, AL_LOOPING, (int)loop);
	}

	void Source::GetOrientation(float* atX, float* atY, float* atZ, float* upX, float* upY, float* upZ) const
	{
		*atX = mOrientation[0];
		*atY = mOrientation[1];
		*atZ = mOrientation[2];
		*upX = mOrientation[3];
		*upY = mOrientation[4];
		*upZ = mOrientation[5];
	}

	void Source::SetOrientation(const float& atX, const float& atY, const float& atZ, const float& upX, const float& upY, const float& upZ)
	{
		if (mStereo)
		{
			LOG_TO_TERMINAL(Logger::Warn, "Stereo audios Spatialization is not supported by OpenAL");
			return;
		}

		alSourcefv(mSourceID, AL_ORIENTATION, mOrientation);
		mOrientation[0] = atX;
		mOrientation[1] = atY;
		mOrientation[2] = atZ;
		mOrientation[3] = upX;
		mOrientation[4] = upY;
		mOrientation[5] = upZ;
	}
}