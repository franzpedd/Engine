#pragma once

#include "Defines.h"
#include <string>

namespace Cosmos
{
	class OpenFileDialog
	{
	public:

		// constructor
		OpenFileDialog() = default;

		// destructor
		~OpenFileDialog() = default;

		// returns the last choosen file path
		inline std::string& GetFilePath() { return mFilePath; }

		// returns the last choosen file name
		inline std::string& GetFileName() { return mFileName; }

	public:

		// shows the file dialog
		void Show();

	private:

		std::string mFilePath = {};
		std::string mFileName = {};
	};

	class SaveFileDialog
	{
	public:

		// constructor
		SaveFileDialog() = default;

		// destructor
		~SaveFileDialog() = default;

		// returns the last choosen file path
		inline std::string& GetFilePath() { return mFilePath; }

		// returns the last choosen file name
		inline std::string& GetFileName() { return mFileName; }

	public:

		// shows the file dialog
		void Show();

	private:

		std::string mFilePath = {};
		std::string mFileName = {};
	};
}