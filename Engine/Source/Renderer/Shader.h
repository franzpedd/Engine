#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>

namespace Cosmos
{
	// forward declarations
	class Device;

	class Shader
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

		// returns a smart pointer to a new vulkan shader
		static std::shared_ptr<Shader> Create(std::shared_ptr<Device> device, Shader::Type type, std::string name, std::string path);

		// constructor
		Shader() = default;

		// destructor
		virtual ~Shader() = default;

	public:

		// returns the shader type
		virtual Shader::Type& GetType() = 0;

		// returns the shader name
		virtual std::string & GetName() = 0;

		// returns the shader path
		virtual std::string& GetPath() = 0;

		// returns the shader module
		virtual VkShaderModule& GetModule() = 0;

		// returns the shader stage info
		virtual VkPipelineShaderStageCreateInfo& GetStage() = 0;
	};
}