#ifndef SGE_ASSERT_HPP_
#define SGE_ASSERT_HPP_

#pragma once

#include <print>

#if SGE_DEBUG
    #include <cstdlib>
    #include <exception>

    #include "defines.hpp"

    #define SGE_ASSERT(expression) do {                                                                 \
        if (!(expression)) {                                                                            \
            std::println(stderr, "Assertion `" #expression "` failed at {}:{}", __FILE__, __LINE__);    \
            SGE_DEBUG_BREAK();                                                                          \
            std::terminate();                                                                           \
        }                                                                                               \
    } while (0)

    #define SGE_ASSERT_M(expression, message, ...) do {                                                                \
        if (!(expression)) {                                                                                           \
            std::println(stderr, "Assertion failed at {}:{}: " message, __FILE__, __LINE__, ##__VA_ARGS__);            \
            SGE_DEBUG_BREAK();                                                                                         \
            std::terminate();                                                                                          \
        }                                                                                                              \
    } while (0)

    #define SGE_UNREACHABLE() do {                                                        \
        std::println(stderr, "Reached unreachable code at {}:{}", __FILE__, __LINE__);    \
        std::terminate();                                                                 \
    } while (0)
#else
    // Silence unused warning with `sizeof(bool(expression)` trick
    #define SGE_ASSERT(expression) ((void)sizeof(bool(expression)))
    #define SGE_ASSERT_M(expression, message) ((void)sizeof(bool(expression)))

    #if defined(__GNUC__) || defined(__clang__)
        #define SGE_UNREACHABLE() __builtin_unreachable()
    #elif defiend(_MSC_VER)
        #define SGE_UNREACHABLE() __assume(false)
    #else
        #error "Unknown compiler; can't define SGE_UNREACHABLE"
    #endif
#endif

#define SGE_PANIC(message, ...) do {                                                     \
    std::println(stderr, "panic at {}:{}: " message, __FILE__, __LINE__, ##__VA_ARGS__); \
    std::terminate();                                                                    \
} while (0)

#endif
