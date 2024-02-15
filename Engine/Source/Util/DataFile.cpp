#include "epch.h"
#include "DataFile.h"

#include <functional>
#include <stack>

namespace Cosmos
{
	DataFile::Writer::Writer(std::ofstream& file, int32_t indentationLevel, std::string indentation, char separator)
		: file(file), indentation(indentation), separator(separator), indentationLevel(0)
	{
	}

	bool DataFile::Write(const DataFile& dataFile, const std::string& path, char separator)
	{
		std::ofstream file(path);
		if (file.is_open())
		{
			Writer writer(file, 0, "\t", separator);
			WriteRecursively(dataFile, writer);

			file.close();
			return true;
		}

		return false;
	}

	bool DataFile::Read(DataFile& dataFile, const std::string& path, char separator)
	{
		std::ifstream file(path);
		if (file.is_open())
		{
			// variables may be outside the loop and we may need to refer to previous iterations
			std::string propName = {};
			std::string propValue = {};

			// using a stack to handle the reading
			// may re-factor this later
			std::stack<std::reference_wrapper<DataFile>> dfStack;
			dfStack.push(dataFile);

			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);

				RemoveWhiteSpaces(line);

				// line is not empty
				if (line.empty())
					continue;

				// test if it's a comment
				if (line[0] == '#')
				{
					DataFile comment;
					comment.mIsComment = true;
					dfStack.top().get().mObjectVec.push_back({ line, comment });

					continue;
				}

				// check if equals symbol exists, if it does it is a property
				size_t x = line.find_first_of('=');
				if (x != std::string::npos)
				{
					propName = line.substr(0, x);
					RemoveWhiteSpaces(propName);

					propValue = line.substr(x + 1, line.size());
					RemoveWhiteSpaces(propValue);

					// elements may contain quotes and separations, must deal with this particularity
					bool inQuotes = false;
					std::string token = {};
					size_t tokenCount = 0;

					for (const auto c : propValue)
					{
						if (c == '\"')
						{
							inQuotes = true;
						}

						else
						{
							// it's in quotes, appends to a string
							if (inQuotes)
							{
								token.append(1, c);
							}

							else
							{
								// char is the separator, register the new property
								if (c == separator)
								{
									RemoveWhiteSpaces(token);
									dfStack.top().get()[propName].SetString(token, tokenCount);

									token.clear();
									tokenCount++;
								}

								// char is part of the token, appends to a string
								else
								{
									token.append(1, c);
								}
							}
						}
					}

					// any left char makes the final token, used to handle any mistake in the file avoiding crashes
					if (!token.empty())
					{
						RemoveWhiteSpaces(token);
						dfStack.top().get()[propName].SetString(token, tokenCount);
					}
				}

				else // no ' = ' sign
				{
					// the previous property is the new node
					if (line[0] == '{')
					{
						dfStack.push(dfStack.top().get()[propName]);
					}

					else
					{
						// node has been finished, pop it from the stack
						if (line[0] == '}')
						{
							dfStack.pop();
						}

						// line is a property with no assignment 
						else
						{
							propName = line;
						}
					}
				}
			}


			file.close();
			return true;
		}

		return false;
	}

	size_t DataFile::GetValueCount() const
	{
		return mContent.size();
	}

	void DataFile::SetString(const std::string& str, const size_t count)
	{
		if (count >= mContent.size())
			mContent.resize(count + 1);

		mContent[count] = str;
	}

	const std::string DataFile::GetString(const size_t count) const
	{
		if (count >= mContent.size())
			return "";

		return mContent[count];
	}

	void DataFile::SetDouble(const double d, const size_t count)
	{
		SetString(std::to_string(d), count);
	}

	const double DataFile::GetDouble(const size_t count) const
	{
		return std::atof(GetString(count).c_str());
	}

	void DataFile::SetInt(const int32_t i, size_t count)
	{
		SetString(std::to_string(i), count);
	}

	const int32_t DataFile::GetInt(size_t count) const
	{
		return std::atoi(GetString(count).c_str());
	}

	void DataFile::WriteRecursively(const DataFile& dataFile, Writer& writer)
	{
		const std::string separatorStr = std::string(1, writer.separator) + " ";

		// iterate through each property of tthis DataFile node
		for (auto const& prop : dataFile.mObjectVec)
		{
			// property doesnt contain any children, so it's an assignment
			if (prop.second.mObjectVec.empty())
			{
				writer.file << Indentation(writer.indentation, writer.indentationLevel) << prop.first << (prop.second.mIsComment ? "" : " = ");

				size_t nItems = prop.second.GetValueCount();
				for (size_t i = 0; i < prop.second.GetValueCount(); i++)
				{
					// ensures the separator is written in quotations if it exists in the list of elements
					size_t x = prop.second.GetString(i).find_first_of(writer.separator);

					if (x != std::string::npos)
					{
						writer.file << "\"" << prop.second.GetString(i) << "\"" << ((nItems > 1) ? separatorStr : "");
					}

					else
					{
						writer.file << prop.second.GetString(i) << ((nItems > 1) ? separatorStr : "");
					}

					nItems--;
				}

				// property handled, move to next line
				writer.file << "\n";
			}

			// property has children
			else
			{
				writer.file << "\n" << Indentation(writer.indentation, writer.indentationLevel) << prop.first << "\n";
				writer.file << Indentation(writer.indentation, writer.indentationLevel) << "{\n";
				writer.indentationLevel++;

				// recusisvely writes that node
				WriteRecursively(prop.second, writer);

				writer.file << Indentation(writer.indentation, writer.indentationLevel) << "}\n\n";
			}
		}

		// decrease indentation for the node
		if (writer.indentationLevel > 0)
		{
			writer.indentationLevel--;
		}
	}

	std::string DataFile::Indentation(const std::string& str, const size_t count)
	{
		std::string res = {};

		for (size_t i = 0; i < count; i++)
		{
			res += str;
		}

		return res;
	}

	void DataFile::RemoveWhiteSpaces(std::string& str)
	{
		str.erase(0, str.find_first_not_of(" \t\n\r\f\v"));
		str.erase(str.find_last_not_of(" \t\n\r\f\v") + 1);
	}
}