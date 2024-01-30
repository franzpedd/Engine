#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26819 26495 28020 )
#endif
#include <nlohmann/json.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using Serializer = nlohmann::json;