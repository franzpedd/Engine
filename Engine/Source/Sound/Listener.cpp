#include "epch.h"
#include "Listener.h"

#include "Debug/Logger.h"

#include <AL/alc.h>
#include <AL/al.h>
#include <AL/alext.h>

namespace Cosmos::sound
{
	Listener& Listener::GetInstance()
	{
		static Listener instance;
		return instance;
	}

	void Listener::GetPosition(float* x, float* y, float* z)
	{
		alGetListener3f(AL_POSITION, x, y, z);
	}

	void Listener::SetPosition(const float& x, const float& y, const float& z)
	{
		alListener3f(AL_POSITION, x, y, z);
	}

	void Listener::GetOrientation(float* orientation)
	{
		alGetListenerf(AL_ORIENTATION, orientation);
	}

	void Listener::SetOrientation(const float& orientation)
	{
		alListenerf(AL_ORIENTATION, orientation);
	}

	void Listener::GetVolume(float* gain)
	{
		alGetListenerf(AL_GAIN, gain);
	}

	void Listener::SetVolume(const float& gain)
	{
		alListenerf(AL_GAIN, gain);
	}

	Listener::Listener()
	{
		// initializes device
		mDevice = alcOpenDevice(nullptr);
		if (!mDevice)
			LOG_TO_TERMINAL(Logger::Assert, "Sound: Failed to get sound device");

		// initializes context
		mContext = alcCreateContext(mDevice, nullptr);
		if (!mContext)
			LOG_TO_TERMINAL(Logger::Assert, "Sound: Failed to create sound context");

		if (!alcMakeContextCurrent(mContext)) 
			LOG_TO_TERMINAL(Logger::Assert, "Sound: Failed to make sound context current");

		// enumerate extensions
		const ALCchar* name = nullptr;

		if (alcIsExtensionPresent(mDevice, "ALC_ENUMERATE_ALL_EXT"))
			name = alcGetString(mDevice, ALC_ALL_DEVICES_SPECIFIER);

		if (!name || alcGetError(mDevice) != AL_NO_ERROR)
			name = alcGetString(mDevice, ALC_DEVICE_SPECIFIER);

		LOG_TO_TERMINAL(Logger::Trace, "Sound: Choosen device: %s", name);
	}

	Listener::~Listener()
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(mContext);
		alcCloseDevice(mDevice);
	}
}