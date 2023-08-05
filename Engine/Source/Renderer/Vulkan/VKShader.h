#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class VKDevice;

	class VKShader
	{
	public:

		enum ShaderType // using shaderc values but only including it on cpp file
		{
			Vertex = 0,
			Fragment = 1,
			Compute = 2,
			Geometry = 3,
			TessControl = 4,
			TessEvaluation = 5,
		};

	public:

		// returns a smart pointer to a new vulkan shader
		static std::shared_ptr<VKShader> Create(std::shared_ptr<VKDevice>& device, ShaderType type, std::string name, std::string path);

		// constructor
		VKShader(std::shared_ptr<VKDevice>& device, ShaderType type, std::string name, std::string path);

		// destructor
		~VKShader();

		// returns the shader type
		inline ShaderType& Type() { return mType; }

		// returns the shader name
		inline std::string& Name() { return mName; }

		// returns the shader path
		inline std::string& Path() { return mPath; }

		// returns the shader module
		inline VkShaderModule& Module() { return mShaderModule; }

		// returns the shader stage info
		inline VkPipelineShaderStageCreateInfo& Stage() { return mShaderStageCI; }

	public:

		// reads and returns shader's binary
		std::vector<char> ReadSPIRV();

	private:

		// compiles and returns a source shader
		std::vector<uint32_t> Compile(const char* source, ShaderType type, bool optimize = false);

		// creates the shader's module of the spir-v binary
		void CreateShaderModule(const std::vector<char>& binary);

		// creates the shaders tage specification
		void CreateShaderStage();

	private:

		std::shared_ptr<VKDevice>& mDevice;
		ShaderType mType;
		std::string mName;
		std::string mPath;
		VkShaderModule mShaderModule = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo mShaderStageCI = {};
	};
}