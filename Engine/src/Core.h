#pragma once

#include <memory>

#ifndef NDEBUG
#define ASH_DEBUG
#endif

#define ASH_LINUX

#ifdef ASH_LINUX
#define ASH_ABORT std::raise(SIGABRT)
#elif ASH_WINDOWS
#define ASH_ABORT __debugbreak()
#endif

#ifdef ASH_DEBUG
#define ENABLE_ASSERTS
#endif

#ifdef ENABLE_ASSERTS
#define ASH_ASSERT(x, ...)                            \
    {                                                 \
        if (!(x)) {                                   \
            ASH_ERROR("Assertion: {0}", __VA_ARGS__); \
            ASH_ABORT;                                \
        }                                             \
    }
#define APP_ASSERT(x, ...)                            \
    {                                                 \
        if (!(x)) {                                   \
            APP_ERROR("Assertion: {0}", __VA_ARGS__); \
            ASH_ABORT;                                \
        }                                             \
    }
#else
#define ASH_ASSERT(x, ...) x
#define APP_ASSERT(x, ...) x
#endif
