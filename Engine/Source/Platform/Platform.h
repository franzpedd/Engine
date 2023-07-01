#pragma once

#include "Core/Defines.h"
#include "Core/Event.h"

#include <map>
#include <memory>
#include <string>

namespace Cosmos
{
	class COSMOS_API Platform
	{
	public:

		// creates a smart-ptr to a platform class
		static std::shared_ptr<Platform> Create(const char* title, u32 width, u32 height, u32 xpos, u32 ypos);

		// creates a surface object on platform-side
		Platform(const char* title, u32 width, u32 height, u32 xpos, u32 ypos);

		// closes the surface object
		~Platform();

		// handle platform messages, usually imput messages
		bool ProcessMessages();

		// freezes the main-thread
		void Sleep(u64 ms);

		// returns current window size
		std::pair<u32, u32> GetSize();

		// sets a new window size
		void SetSize(std::pair<u32, u32> size);

	private:

		const char* m_Title;
		u32 m_Width;
		u32 m_Height;
		u32 m_XPos;
		u32 m_YPos;
	};
	
	// returns a vector of files read in a directory
	COSMOS_API void ReadDirectory(const std::string& directory, const std::string& pattern, std::map<std::string, std::string>& files, bool recursive);

	// writes a message to the application's console
	COSMOS_API void OutputMessageToConsole(const char* message, u8 color);

	// writes an error to the application's console
	COSMOS_API void OutputErrorToConsole(const char* message, u8 color);

	// returns the current time
	COSMOS_API f64 GetTime();
}