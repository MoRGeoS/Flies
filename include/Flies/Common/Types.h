#pragma once

#include <cstdint>
#include <limits>

namespace Flies
{
	using size_type = uint32_t;
	constexpr size_type minSize = std::numeric_limits<size_type>::min();
	constexpr size_type maxSize = std::numeric_limits<size_type>::max();
}
