#include "epch.h"

// pre-compiles the tinyGLTF and STB implementation
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26495 26819 28020 6262 4267 4018 )
#endif
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>
#if defined(_MSC_VER)
#pragma warning( pop)
#endif