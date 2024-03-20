#include "epch.h"
#include "Source.h"

#include "Thread/Pool.h"
#include <AL/al.h>

namespace Cosmos::sound
{
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

	std::string& Source::GetPath()
	{
		return mPath;
	}

	uint32_t Source::GetBufferID() const
	{
		return mBufferID;
	}

	uint32_t Source::GetSourceID() const
	{
		return mSourceID;
	}

	const Source::Status Source::GetStatus() const
	{
		return mStatus;
	}

	void Source::Create(std::string path)
	{
		mPath = path;

		// clean previous sound if exists
		if (alIsSource(mSourceID))
		{
			if(mStatus != Stopped)
				Stop();

			alDeleteSources(1, &mSourceID);

			if (mBufferID && alIsBuffer(mBufferID))
			{
				alDeleteBuffers(1, &mBufferID);
			}
		}

		// upload the wave file and create the sound using a separate thread
		mWavInfo = {};
		mWavInfo.path = mPath;
		mWavInfo.pitch = mPitch;
		mWavInfo.volume = mVolume;
		mWavInfo.position = mPosition;
		mWavInfo.velocity = mVelocity;
		mWavInfo.at = mAt;
		mWavInfo.up = mUp;
		mWavInfo.looping = mLoop;
		mWavInfo.stereo = mStereo;
		mAsyncCall = thread::PoolManager::GetInstance().GetResourcesPool()->Enqueue(thread::UploadWaveFile, &mWavInfo, &mBufferID, &mSourceID);
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

	glm::vec3 Source::GetPosition() const
	{
		return mPosition;
	}

	void Source::SetPosition(const glm::vec3 position, bool changeImmediatly)
	{
		if (mStereo)
		{
			LOG_TO_TERMINAL(Logger::Warn, "Stereo audios Spatialization is not supported by OpenAL");
			return;
		}

		mPosition = position;

		if(changeImmediatly)
			alSource3f(mSourceID, AL_POSITION, position.x, position.y, position.z);
	}

	glm::vec3 Source::GetVelocity() const
	{
		return mVelocity;
	}

	void Source::SetVelocity(const glm::vec3 velocity, bool changeImmediatly)
	{
		if (mStereo)
		{
			LOG_TO_TERMINAL(Logger::Warn, "Stereo audios Spatialization is not supported by OpenAL");
			return;
		}

		mVelocity = velocity;

		if (changeImmediatly)
			alSource3f(mSourceID, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	}

	float Source::GetVolume() const
	{
		return mVolume;
	}

	void Source::SetVolume(const float& volume, bool changeImmediatly)
	{
		mVolume = volume;

		if (changeImmediatly)
			alSourcef(mSourceID, AL_GAIN, volume);
	}

	bool Source::GetLooping() const
	{
		return mLoop;
	}

	void Source::SetLooping(bool loop, bool changeImmediatly)
	{
		mLoop = loop;

		if (changeImmediatly)
			alSourcei(mSourceID, AL_LOOPING, (int)loop);
	}

	std::pair<glm::vec3, glm::vec3> Source::GetOrientation() const
	{
		return std::make_pair(mAt, mUp);
	}

	void Source::SetOrientation(const glm::vec3& at, const glm::vec3& up, bool changeImmediatly)
	{
		if (mStereo)
		{
			LOG_TO_TERMINAL(Logger::Warn, "Stereo audios Spatialization is not supported by OpenAL");
			return;
		}

		mAt = at;
		mUp = up;
		
		if (changeImmediatly)
		{
			float orientation[6] = {};
			orientation[0] = at.x;
			orientation[1] = at.y;
			orientation[2] = at.z;
			orientation[3] = up.x;
			orientation[4] = up.y;
			orientation[5] = up.z;
			
			alSourcefv(mSourceID, AL_ORIENTATION, orientation);
		}
	}
}