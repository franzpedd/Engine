#include "epch.h"
#include "Material.h"

namespace Cosmos
{
    Material::Material(std::string name)
    {
        mSpecification.name = name;
    }

    Material::~Material()
    {
        
    }
}