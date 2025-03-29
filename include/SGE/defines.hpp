#ifndef _SGE_DEFINES_HPP_
#define _SGE_DEFINES_HPP_

#define _SGE_BEGIN namespace sge {
#define _SGE_END }

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__NT__)
    #define SGE_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define SGE_PLATFORM_MACOS 1
#elif defined(__linux__)
    #define SGE_PLATFORM_LINUX 1
#else
    #error "Unknown platform"
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define SGE_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define SGE_FORCE_INLINE __forceinline
#else
    #error "Unknown compiler; can't define SGE_FORCE_INLINE"
#endif

#if SGE_PLATFORM_WINDOWS
    #define SGE_DEBUG_BREAK() __debugbreak()
#else
    #include <signal.h>
    #define SGE_DEBUG_BREAK() raise(SIGTRAP)
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define SGE_ALIGN(x) __attribute__ ((aligned(x)))
#elif defined(_MSC_VER)
    #define SGE_ALIGN(x) __declspec(align(x))
#else
    #error "Unknown compiler; can't define ALIGN"
#endif

#endif