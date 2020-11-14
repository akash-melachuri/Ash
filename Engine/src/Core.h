#pragma once

#ifndef NDEBUG
    #define ENABLE_ASSERTS
#endif

#ifdef ENABLE_ASSERTS
	#define ASH_ASSERT(x, ...) {if(!x) {ASH_ERROR("Assertion: {0}", __VA_ARGS__); __debugbreak();}}  
	#define APP_ASSERT(x, ...) {if(!x) {APP_ERROR("Assertion: {0}", __VA_ARGS__); __debugbreak();}}
#elif
	#define ASH_ASSERT(x, ...)
	#define APP_ASSERT(x, ...) 
#endif
