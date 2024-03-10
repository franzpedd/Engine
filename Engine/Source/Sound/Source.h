#pragma once

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
		Source(const char* path);

		// destructor
		~Source();

		// returns sound's buffer id on the OpenAL backend side
		unsigned int GetBufferID() const;

		// returns sound's source id on the OpenAL backend side
		unsigned int GetSourceID() const;

		// returns sound's path on disk
		const char* GetPath() const;

		// returns the sound current status
		const Source::Status GetStatus() const;

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
		void GetPosition(float* x, float* y, float* z) const;

		// changes the position of the source
		void SetPosition(const float& x, const float& y, const float& z);

		// returns the position of the source
		void GetVelocity(float* x, float* y, float* z) const;

		// changes the velocity of the source
		void SetVelocity(const float& x, const float& y, const float& z);

		// returns the sound current volume
		float GetVolume() const;

		// changes the sound current volume
		void SetVolume(const float& volume);

		// returns if the sound is on looping mode
		bool GetLooping() const;

		// changes the sound to looping mode
		void SetLooping(bool loop);

		// returns the sound orientation
		void GetOrientation(float* atX, float* atY, float* atZ, float* upX, float* upY, float* upZ) const;

		// changes the sound orientation
		void SetOrientation(const float& atX, const float& atY, const float& atZ, const float& upX, const float& upY, const float& upZ);

	private:

		const char* mPath;
		bool mStereo = true;

		unsigned int mBufferID = 0;
		unsigned int mSourceID = 0;
		Status mStatus = Initial;

		float mPitch = 1.0f;
		float mVolume = 1.0f;
		float mPosition[3] = { 0.0f, 0.0f, 0.0f };
		float mVelocity[3] = { 0.0f, 0.0f, 0.0f };
		bool mLoop = false;

		//			 		      at.x  at.y  at.z   up.x  up.x  up.z
		float mOrientation[6] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
	};
}