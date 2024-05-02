#pragma once

#include "Renderer/Texture.h"
#include "Util/Math.h"
#include <string>

namespace Cosmos
{
    class Material
    {
    public:

        // alpha channel blending mode
        struct Specification
        {
            enum Alphamode 
            {
                Opaque, 
                Mask, 
                Blend 
            };
            
            std::string name;
            Alphamode mode; // how to handle the alpha channel
            float alphaCutoff = 1.0f; // amout of oppacity
            bool culling = false; // this is doubleSided on GLTF specification, enabling back-culling when false
            bool extUnlit = false; // extension that unlits the material
            float extEmissiveStrength = 1.0f; // extension that adds emissive strength on the material
        };

        // material base color information, only the image's pixels
        struct Albedo
        {
            bool enabled = false;
            Shared<Texture2D> texture;
            glm::vec4 factor = glm::vec4(1.0f); // base color
        };

        // simulates 3d details on 2d surface manipulating lightning
        struct Normal
        {
            bool enabled = false;
            Shared<Texture2D> texture;
        };

        // simulates light shaddows without a direct source light
        struct Occlusion
        {
            bool enabled = false;
            Shared<Texture2D> texture;
        };

        // simulates glowing effect within the texture
        struct Emissive
        {
            bool enabled = false;
            Shared<Texture2D> texture;
            glm::vec4 factor = glm::vec4(0.0f); // effect's default stregth

        };

        // default material workflow, can be turned off in favor of SpecularGlossiness
        struct MetallicRoughness
        {
            bool enabled = false;
            Shared<Texture2D> texture;
            float mettalic = 1.0f; // mettalic factor intensity
            float roughness = 1.0f; // roughness factor intensity
        };

        // extension workflow, can be choosen instead of metallic-roughness if available
        struct SpecularGlossiness
        {
            bool enabled = false;
            Shared<Texture2D> specular;
            Shared<Texture2D> diffuse;
            glm::vec4 specularFactor = glm::vec4(1.0f); // specular intensity
            glm::vec4 diffuseFactor = glm::vec4(1.0f); // diffuse intesity
        };

    public:

        // constructor
        Material(std::string name = "Material");

        // destructor
        ~Material();

        // returns a reference to the material specification
        inline Specification& GetSpecificationRef() { return mSpecification; }

        // returns a reference to the mateiral albedo properties
        inline Albedo& GetAlbedoPropertiesRef() { return mAlbedoProperties; }

        // returns a reference to the material normal properties
        inline Normal& GetNormalPropertiesRef() { return mNormalProperties; }

        // returns a reference to the material occlusion properties
        inline Occlusion& GetOcclusionPropertiesRef() { return mOcclusionProperties; }

        // returns a reference to the material emissive properties
        inline Emissive& GetEmissivePropertiesRef() { return mEmissiveProperties; }

        // returns a reference to the material metallic-roughness workflow
        inline MetallicRoughness& GetMetallicRoughnessRef() { return mMetallicRoughnessProperties; }

        // returns a reference to the material specular-glossiness workflow
        inline SpecularGlossiness& GetSpecularGlossinessRef() { return mSpecularGlossinessProperties; }
    
    private:

        Specification mSpecification;
        Albedo mAlbedoProperties;
        Normal mNormalProperties;
        Occlusion mOcclusionProperties;
        Emissive mEmissiveProperties;
        MetallicRoughness mMetallicRoughnessProperties;
        SpecularGlossiness mSpecularGlossinessProperties;
    };
}