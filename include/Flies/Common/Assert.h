#pragma once

#include "Macros.h"

// If positive debug level
#if FLS_DEBUG_LEVEL >= 0

#	include <format>
#	include <iostream>

#	define FLS_INTERNAL_ASSERT_IMPL(cond, message) if (!(cond)) { std::clog << message << std::endl; FLS_DEBUG_BREAK(); }
#	define FLS_INTERNAL_ASSERT_IMPL_FMT(cond, message, ...) if (!(cond)) { std::clog << std::format(message, __VA_ARGS__) << std::endl; FLS_DEBUG_BREAK(); }

#	define FLS_INTERNAL_ASSERT_WITH_MSG_FMT(cond, message, ...) FLS_INTERNAL_ASSERT_IMPL(cond, "Assertion failed: {}", std::format(message, __VA_ARGS__))
#	define FLS_INTERNAL_ASSERT_WITH_MSG(cond, message) FLS_INTERNAL_ASSERT_IMPL(cond, message)
#	define FLS_INTERNAL_ASSERT_NO_MSG(cond) FLS_INTERNAL_ASSERT_IMPL(cond, "Assertion failed at {}:{}", __FILE__, __LINE__)

#	define FLS_INTERNAL_ASSERT_GET_MACRO_NAME(_1, _2, _3, NAME, ...) NAME
#	define FLS_INTERNAL_ASSERT_GET_MACRO(...) FLS_EXPAND_MACRO( FLS_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, FLS_INTERNAL_ASSERT_WITH_MSG_FMT, FLS_INTERNAL_ASSERT_WITH_MSG, FLS_INTERNAL_ASSERT_NO_MSG) )

#	define FLS_ASSERT(...) FLS_EXPAND_MACRO( FLS_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(__VA_ARGS__) )
#else
#	define FLS_ASSERT(...)
#endif
