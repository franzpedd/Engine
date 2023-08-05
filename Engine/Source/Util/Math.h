#pragma once

// todo: encapsulate glm into custom syntax to facilitate transition in the future
// also, visual studio stupidity 2

#define GLM_FORCE_RADIANS

#if defined(_MSC_VER)
	#pragma warning( push )
	#pragma warning( disable : 26495)
#endif
#include <glm/glm.hpp>
#if defined(_MSC_VER)
	# pragma warning(pop)
#endif
