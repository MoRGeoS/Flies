#pragma once

// No-debug: level = -1
// Undefined debug: level = 0
// Defined debug: level = 1

#ifdef NDEBUG
#	define FLS_DEBUG_LEVEL -1
#elif !defined(_DEBUG)
#	define FLS_DEBUG_LEVEL 0
#else
#	define FLS_DEBUG_LEVEL 1
#endif

#define FLS_DEBUG_BREAK() __debugbreak()
#define FLS_EXPAND_MACRO(x) x
#define FLS_STRINGIFY(x) #x
