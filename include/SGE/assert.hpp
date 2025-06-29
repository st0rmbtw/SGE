#ifndef _SGE_ASSERT_HPP_
#define _SGE_ASSERT_HPP_

#pragma once

#if SGE_DEBUG
    #include <cstdlib>
    #include <fmt/core.h>
    #include <fmt/ostream.h>

    #define SGE_ASSERT(expression)                                                                      \
        if (!(expression)) {                                                                            \
            fmt::println(stderr, "Assertion `" #expression "` failed at {}:{}", __FILE__, __LINE__);    \
            std::abort();                                                                               \
        }

    #define SGE_UNREACHABLE() {                                                           \
        fmt::println(stderr, "Reached unreachable code at {}:{}", __FILE__, __LINE__);    \
        std::abort();                                                                     \
    }
#else
    #define SGE_ASSERT(expression) ((void)0)

    #if defined(__GNUC__) || defined(__clang__)
        #define SGE_UNREACHABLE() __builtin_unreachable()
    #elif defiend(_MSC_VER)
        #define SGE_UNREACHABLE() __assume(false)
    #else
        #error "Unknown compiler; can't define SGE_UNREACHABLE"
    #endif

#endif



#endif
