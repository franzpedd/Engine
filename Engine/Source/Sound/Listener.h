#pragma once


// forward declarations
struct ALCdevice;
struct ALCcontext;

namespace Cosmos::sound
{
	// The listener class will represent the headset/speaker
	// OpenAL only supports one listener and thus, this class is a singleton
	// Although OpenAL supports multiple devices, we'll probably be sticking with only one device
	// and thus, the listener also initialize/deinitialize the OpenAL device and context backends
	class Listener
	{
	public:

		// delete copy and move constructors and assign operators
		Listener(Listener const&) = delete;             // copy construct
		Listener(Listener&&) = delete;                  // move construct
		Listener& operator=(Listener const&) = delete;  // copy assign
		Listener& operator=(Listener&&) = delete;		// move assign

		// returns the listener singleton
		static Listener& GetInstance();

	public:

		// returns listener's location
		void GetPosition(float* x, float* y, float* z);

		// sets a new location for the listener
		void SetPosition(const float& x, const float& y, const float& z);

		// returns listener's orientation
		void GetOrientation(float* orientation);

		// sets a new orientation for the listener
		void SetOrientation(const float& orientation);

		// returns listener's volume/gain
		void GetVolume(float* gain);

		// sets a new volume/gain for the listener
		void SetVolume(const float& gain);

	protected:

		// constructor
		Listener();

		// destructor
		~Listener();

	private:

		static Listener* sListener;

		ALCdevice* mDevice = nullptr;
		ALCcontext* mContext = nullptr;
	};
}