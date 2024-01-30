#include "epch.h"

// pre-compiles STB implementation
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26495 26819 28020 6262 4267 4018 )
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#if defined(_MSC_VER)
#pragma warning( pop)
#endif