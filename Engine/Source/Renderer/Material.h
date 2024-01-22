#pragma once

// VkDescriptorSet and it's related structs are the only vulkan object used on this .h/.cpp file
// there's no equivalent in DX12 or Metal therefore may be worthy creating a wrapper in the future
#include <vulkan/vulkan.h>
#include "Util/Math.h"

namespace Cosmos
{
	// forward declarations
	class Texture2D;

	class Material
	{
	public:

		typedef enum class AlphaMode
		{
			ALPHAMODE_OPAQUE = 0,
			ALPHAMODE_MASK,
			ALPHAMODE_BLEND
		} AlphaMode;

		typedef enum class Workflow
		{
			METALLIC_ROUGHNESS = 0,
			SPECULAR_GLOSSINESS
		} Workflow;

		struct CoordinateSets
		{
			uint8_t baseColor = 0;
			uint8_t metallicRoughness = 0;
			uint8_t normalMap = 0;
			uint8_t occlusionMap = 0;
			uint8_t emissiveMap = 0;
			uint8_t specularGlossiness = 0; // only used for Specular Glossiness
		};

		struct SpecularGlossiness // KHR_materials_pbrSpecularGlossiness
		{
			glm::vec4 specularFactor = glm::vec4(0.0f);
			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			std::shared_ptr<Texture2D> specularGlossiness;
			std::shared_ptr<Texture2D> diffuseMap;
		};

	public:

		// constructor
		Material();

		// destructor
		~Material();

	public:

		// returns the used descriptor set for the material
		inline VkDescriptorSet& GetDescriptorSet() { return mDescriptorSet; }

		// returns the texture coordinates found when deserializing the material
		inline CoordinateSets& GetCoordinateSets() { return mTextureCoordinateSets; }

		// returns the material current PBR workflow
		inline Workflow& GetWorkflow() { return mWorkflow; }

	public:

		// returns current alpha mode
		inline AlphaMode& GetAlphaMode() { return mAlphaMode; }

		// returns current alpha cutoff
		inline float& GetAlphaCutoff() { return mAlphaCutoff; }

		// returns current metallic factor
		inline float& GetMetallicFactor() { return mMetallicFactor; }

		// returns current roughness factor
		inline float& GetRoughnessFactor() { return mRoughnessFactor; }

		// returns current base color
		inline glm::vec4& GetBaseColorFactor() { return mBaseColorFactor; }

		// returns current emissive factor
		inline glm::vec4& GetEmissiveFactor() { return mEmissiveFactor; }

		// returns base texture
		inline std::shared_ptr<Texture2D>& GetBaseTexture() { return mBaseTexture; }

		// returns metallic roughness texture
		inline std::shared_ptr<Texture2D>& GetMetallicRoughnessTexture() { return mMetallicRoughness; }

		// returns normal map
		inline std::shared_ptr<Texture2D>& GetNormalMap() { return mNormalMap; }

		// returns occlusion map
		inline std::shared_ptr<Texture2D>& GetOcclusionMap() { return mOcclusionMap; }

		// returns emissive map
		inline std::shared_ptr<Texture2D>& GetEmissiveMap() { return mEmissiveMap; }

		// returns the specular glossiness extension related objects
		inline SpecularGlossiness& GetSpecularGlossiness() { return mSpecularGlossiness; }

	private:

		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
		Workflow mWorkflow = Workflow::METALLIC_ROUGHNESS;
		AlphaMode mAlphaMode = AlphaMode::ALPHAMODE_OPAQUE;
		float mAlphaCutoff = 1.0f;
		float mMetallicFactor = 1.0f;
		float mRoughnessFactor = 1.0f;
		glm::vec4 mBaseColorFactor = glm::vec4(1.0f);
		glm::vec4 mEmissiveFactor = glm::vec4(1.0f);
		std::shared_ptr<Texture2D> mBaseTexture;
		std::shared_ptr<Texture2D> mMetallicRoughness;
		std::shared_ptr<Texture2D> mNormalMap;
		std::shared_ptr<Texture2D> mOcclusionMap;
		std::shared_ptr<Texture2D> mEmissiveMap;
		CoordinateSets mTextureCoordinateSets = {};

		// specular-glossiness (considered as an extension and may be null if not used )
		SpecularGlossiness mSpecularGlossiness = {};
	};
}