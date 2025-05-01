#ifndef _SGE_DEFINES_HPP_
#define _SGE_DEFINES_HPP_

#define _SGE_BEGIN namespace sge {
#define _SGE_END }

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__NT__)
    #define SGE_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define SGE_PLATFORM_APPLE 1

    #include <TargetConditionals.h>
    
    #if TARGET_OS_OSX
        #define SGE_PLATFORM_MACOS 1
    #elif TARGET_OS_IOS
        #define SGE_PLATFORM_IOS 1
    #elif TARGET_OS_TV
        #define SGE_PLATFORM_TVOS 1
    #elif TARGET_OS_WATCH
        #define SGE_PLATFORM_WATCHOS 1
    #elif TARGET_OS_BRIDGE
        #define SGE_PLATFORM_BRIDGEOS 1
    #endif


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
