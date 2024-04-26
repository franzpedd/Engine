#pragma once

#include "VKDefines.h"
#include "Util/Memory.h"
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKDevice;

	class VKShader
	{
	public:

		enum Type // using shaderc values but only including it on cpp file
		{
			Vertex = 0,
			Fragment = 1,
			Compute = 2,
			Geometry = 3,
			TessControl = 4,
			TessEvaluation = 5
		};

	public:

		// constructor
		VKShader(Shared<VKDevice> device, Type type, std::string name, std::string path);

		// destructor
		~VKShader();

		// returns the shader type
		inline Type GetType() { return mType; }

		// returnsa reference to the shader name
		inline std::string& GetNameRef() { return mName; }

		// returns a reference to the shader path
		inline std::string& GetPathRef() { return mPath; }

		// returns the shader module
		inline VkShaderModule GetModule() { return mShaderModule; }

		// returns a reference to the shader stage info
		VkPipelineShaderStageCreateInfo& GetShaderStageCreateInfoRef() { return mShaderStageCI; }

	private:

		// reads and returns shader's binary
		std::vector<char> ReadSPIRV();

		// compiles and returns a source shader
		std::vector<uint32_t> Compile(const char* source, Type type, bool optimize = false);

		// creates the shader's module of the spir-v binary
		void CreateShaderModule(const std::vector<char>& binary);

		// creates the shaders tage specification
		void CreateShaderStage();

	private:

		Shared<VKDevice> mDevice;
		Type mType;
		std::string mName;
		std::string mPath;
		VkShaderModule mShaderModule = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo mShaderStageCI = {};
	};
}