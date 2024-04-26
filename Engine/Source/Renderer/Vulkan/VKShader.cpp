#include "epch.h"
#include "VKShader.h"

#include "VKDevice.h"

// stupid visual studio propagating warnings from thirdparty libraries
#if defined(_MSC_VER)
	#pragma warning( push )
	#pragma warning( disable : 26439)
#endif
#include <shaderc/shaderc.hpp>
#if defined(_MSC_VER)
	# pragma warning(pop)
#endif

namespace Cosmos
{
	VKShader::VKShader(Shared<VKDevice> device, Type type, std::string name, std::string path)
		: mDevice(device), mType(type), mName(name), mPath(path)
	{
		Logger() << "Creating VKShader";

		// reads raw shader
		std::ifstream read(path);
		std::stringstream buffer;

		buffer << read.rdbuf();
		read.close();

		std::string auxname = path;
		auxname += ".spv";

		// writes compiled shared
		std::vector<uint32_t> bin = Compile(buffer.str().c_str(), type, false);
		std::ofstream write(auxname.c_str(), std::ios::out | std::ios::binary);

		write.write(reinterpret_cast<char*>(&bin[0]), bin.size() * sizeof(bin[0]));
		write.close();

		CreateShaderModule(ReadSPIRV());
		CreateShaderStage();
	}

	VKShader::~VKShader()
	{
		vkDestroyShaderModule(mDevice->GetDevice(), mShaderModule, nullptr);
	}

	std::vector<char> VKShader::ReadSPIRV()
	{
		std::string name = mPath;
		name += ".spv";

		std::ifstream bin(name, std::ios::ate | std::ios::binary);

		size_t fileSize = (size_t)bin.tellg();
		std::vector<char> buffer(fileSize);

		bin.seekg(0);
		bin.read(buffer.data(), fileSize);
		bin.close();

		return buffer;
	}

	std::vector<uint32_t> VKShader::Compile(const char* source, Type type, bool optimize)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_size);
		}

		shaderc::SpvCompilationResult res = compiler.CompileGlslToSpv(source, (shaderc_shader_kind)type, mPath.c_str(), options);

		if (res.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_TO_TERMINAL(Logger::Severity::Assert, "Failed to Compile shader %s. Details: %s", mPath.c_str(), res.GetErrorMessage().c_str());
		}

		return { res.cbegin(), res.cend() };
	}

	void VKShader::CreateShaderModule(const std::vector<char>& binary)
	{
		VkShaderModuleCreateInfo shaderModuleCI = {};
		shaderModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCI.pNext = nullptr;
		shaderModuleCI.flags = 0;
		shaderModuleCI.codeSize = binary.size();
		shaderModuleCI.pCode = reinterpret_cast<const uint32_t*>(binary.data());

		VK_ASSERT(vkCreateShaderModule(mDevice->GetDevice(), &shaderModuleCI, nullptr, &mShaderModule), "Failed to create shader module");
	}

	void VKShader::CreateShaderStage()
	{
		mShaderStageCI = {};
		mShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		mShaderStageCI.pNext = nullptr;
		mShaderStageCI.flags = 0;
		mShaderStageCI.pName = "main";
		mShaderStageCI.module = mShaderModule;
		mShaderStageCI.pSpecializationInfo = nullptr;

		switch (mType)
		{
			case Type::Vertex:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			}

			case Type::Fragment:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			}

			case Type::Compute:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
				break;
			}

			case Type::Geometry:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				break;
			}

			case Type::TessControl:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
				break;
			}

			case Type::TessEvaluation:
			{
				mShaderStageCI.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
				break;
			}
		}
	}
}