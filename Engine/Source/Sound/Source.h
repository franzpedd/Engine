#pragma once

// used for uploading the wave file asynchronously
#include "Thread/AsyncResource.h"
#include "Util/Math.h"

#include <future>

namespace Cosmos::sound
{
	// Sources are sounds that exists on the program, they are spatialized and is possible to configure it's property
	// They are played on a separated thread on the backend side, but beware of how many sound sources exists on a given time
	// The constructor uploads the sound buffer to the device and it stays there until it is deconstructed
	// For huge files consider create SourceStream class and implement the loading of parts, but this is not needed right now
	class Source
	{
	public:

		// all available status a sound may be at (only on status at the time)
		typedef enum Status
		{
			Initial = 0,
			Playing,
			Paused,
			Stopped
		} Status;

	public:

		// constructor
		Source() = default;

		// destructor
		~Source();

		// returns sound's path on disk
		std::string& GetPath();

		// returns sound's buffer id on the OpenAL backend side
		uint32_t GetBufferID() const;

		// returns sound's source id on the OpenAL backend side
		uint32_t GetSourceID() const;

		// returns the sound current status
		const Source::Status GetStatus() const;

	public:

		// creates or recreates the sound given a path
		void Create(std::string path);

	public: // controlls the sound status

		// starts playing the sound
		void Start();

		// stops the sound
		void Stop();

		// pauses the sound
		void Pause();

		// unpauses the sound (starts from stopped or begining if status was stopped)
		void Resume();

		// Rewinds the sound (and restarts it if desired)
		void Rewind(bool restart = false);

	public: // controll various aspects of the sound sorce

		// returns the position of the source
		glm::vec3 GetPosition() const;

		// changes the position of the source
		void SetPosition(const glm::vec3 position, bool changeImmediatly = true);

		// returns the position of the source
		glm::vec3 GetVelocity() const;

		// changes the velocity of the source
		void SetVelocity(const glm::vec3 velocity, bool changeImmediatly = true);

		// returns the sound current volume
		float GetVolume() const;

		// changes the sound current volume
		void SetVolume(const float& volume, bool changeImmediatly = true);

		// returns if the sound is on looping mode
		bool GetLooping() const;

		// changes the sound to looping mode
		void SetLooping(bool loop, bool changeImmediatly = true);

		// returns the sound orientation
		std::pair<glm::vec3, glm::vec3> GetOrientation(void) const;

		// changes the sound orientation
		void SetOrientation(const glm::vec3& at, const glm::vec3& up, bool changeImmediatly = true);

	private:

		Source::Status mStatus = Stopped;
		std::string mPath = {};
		uint32_t mBufferID = 0;
		uint32_t mSourceID = 0;
		float mPitch = 1.0f;
		float mVolume = 1.0f;
		bool mLoop = false;
		bool mStereo = true;
		glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 mVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 mAt = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);

		std::future<thread::Status> mAsyncCall;
		thread::WaveAsync::Arguments mWavInfo;
	};
}