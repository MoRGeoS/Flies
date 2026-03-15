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

#if defined(_MSC_VER)
#	define FLS_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
#	if __has_builtin(__builtin_debugtrap)
#		define FLS_DEBUG_BREAK() __builtin_debugtrap()
#	else
#		define FLS_DEBUG_BREAK() __asm__ volatile("int3")
#	endif
#elif defined(__GNUC__)
#	if defined(__i386__) || defined(__x86_64__)
#		define FLS_DEBUG_BREAK() __asm__ volatile("int3")
#	elif defined(__aarch64__) || defined(__arm__)
#		define FLS_DEBUG_BREAK() __asm__ volatile("brk #0")
#	else
#		include <signal.h>
#		define FLS_DEBUG_BREAK() raise(SIGTRAP)
#	endif
#else
#	include <signal.h>
#	define FLS_DEBUG_BREAK() raise(SIGTRAP)
#endif

#define FLS_EXPAND_MACRO(x) x
#define FLS_STRINGIFY(x) #x
