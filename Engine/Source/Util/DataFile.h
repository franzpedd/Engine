#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>

namespace Cosmos
{
	class DataFile
	{
	private:

		// helper to write to files
		struct Writer
		{
			std::ofstream& file;
			int32_t indentationLevel;
			std::string indentation;
			char separator;

			// constructor
			Writer(std::ofstream& file, int32_t indentationLevel = 0, const std::string indentation = "\t", char separator = ',');
		};

	public: // functions

		// writes a data file to a file
		static bool Write(const DataFile& dataFile, const std::string& path, char separator = ',');

		// reads data from a file
		static bool Read(DataFile& dataFile, const std::string& path, char separator = ',');

	public: // operator overloading

		inline DataFile& operator[](const std::string& name)
		{
			// node map already does not contains an object with this name
			if (mObjectMap.count(name) == 0)
			{
				// create the object in the map and create a new empty DataFile on the object vector
				mObjectMap[name] = mObjectVec.size();
				mObjectVec.push_back({ name, DataFile() });
			}

			// returns the object by it's map index
			return mObjectVec[mObjectMap[name]].second;
		}

	public: // utils

		// returns the number of values a property has
		size_t GetValueCount() const;


	public: // getters and setters

		// sets a new string value of a property
		void SetString(const std::string& str, const size_t count = 0);

		// returns a string value of a property
		const std::string GetString(const size_t count = 0) const;

		// sets a new double value of a property
		void SetDouble(const double d, const size_t count = 0);

		// returns the double value of a property
		const double GetDouble(const size_t count = 0) const;

		// sets a new integer value of a property
		void SetInt(const int32_t i, size_t count = 0);

		// returns the integer value of a property 
		const int32_t GetInt(size_t count = 0) const;

	private:

		// recursively writes to a data file to a file
		static void WriteRecursively(const DataFile& dataFile, Writer& writer);

		// returns the indentation level stringified
		static std::string Indentation(const std::string& str, const size_t count);

		// remvoes the white spaces of a string
		static void RemoveWhiteSpaces(std::string& str);

	protected:

		bool mIsComment = false; // used to identify if the property is a comment or not

	private:

		std::vector<std::string> mContent; // the items of this serializer

		std::vector<std::pair<std::string, DataFile>> mObjectVec; // child nodes of this datafile
		std::unordered_map<std::string, size_t>  mObjectMap;
	};
}