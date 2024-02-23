#pragma once

#include "Renderer/Shader.h"
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKDevice;

	class VKShader : public Shader
	{
	public:

		// returns a smart pointer to a new vulkan shader
		static std::shared_ptr<VKShader> Create(std::shared_ptr<VKDevice> device, Shader::Type type, std::string name, std::string path);

		// constructor
		VKShader(std::shared_ptr<VKDevice> device, Shader::Type type, std::string name, std::string path);

		// destructor
		virtual ~VKShader() = default;

		// returns the shader type
		virtual Shader::Type& GetType() override;

		// returns the shader name
		virtual std::string& GetName() override;

		// returns the shader path
		virtual std::string& GetPath() override;

		// returns the shader module
		virtual VkShaderModule& GetModule() override;

		// returns the shader stage info
		virtual VkPipelineShaderStageCreateInfo& GetStage() override;

	private:

		// reads and returns shader's binary
		std::vector<char> ReadSPIRV();

		// compiles and returns a source shader
		std::vector<uint32_t> Compile(const char* source, Shader::Type type, bool optimize = false);

		// creates the shader's module of the spir-v binary
		void CreateShaderModule(const std::vector<char>& binary);

		// creates the shaders tage specification
		void CreateShaderStage();

	private:

		std::shared_ptr<VKDevice> mDevice;
		Shader::Type mType;
		std::string mName;
		std::string mPath;
		VkShaderModule mShaderModule = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo mShaderStageCI = {};
	};
}