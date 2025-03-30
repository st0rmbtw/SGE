#ifndef _SGE_ASSERT_HPP_
#define _SGE_ASSERT_HPP_

#pragma once

#if SGE_DEBUG
    #include <stdio.h>
    #include <stdlib.h>
    #define SGE_ASSERT(expression, message, ...)                                                    \
        do {                                                                                    \
            if (!(expression)) {                                                                \
                fprintf(stderr, "[%s:%d] " message "\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
                abort();                                                                        \
            }                                                                                   \
        } while (0)
        
    #define SGE_UNREACHABLE() SGE_ASSERT(false, "Reached an unreachable point!")
#else
    #define SGE_ASSERT(expression, message, ...) ((void)0)

    #if defined(__GNUC__) || defined(__clang__)
        #define SGE_UNREACHABLE() __builtin_unreachable()
    #elif defiend(_MSC_VER)
        #define SGE_UNREACHABLE() __assume(false)
    #else
        #error "Unknown compiler; can't define SGE_UNREACHABLE"
    #endif

#endif



#endif
