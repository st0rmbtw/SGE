#ifndef _SGE_UTILS_RANDOM_HPP_
#define _SGE_UTILS_RANDOM_HPP_

#include <cstdlib>

#include <SGE/defines.hpp>
#include <SGE/assert.hpp>

_SGE_BEGIN

namespace random {

/**
 * @brief Generates a random integer within a given range
 * 
 * @param from Specifies the start of the range (inclusive)
 * @param to Specifies the end of the range (inclusive)
 * @return Generated random integer
 */
inline int rand_int(int from, int to) {
    SGE_ASSERT(from < to);
    return from + rand() % (to + 1 - from);
}

/**
 * @brief Generates a random float within a given range
 * 
 * @param from Specifies the start of the range (inclusive)
 * @param to Specifies the end of the range (inclusive)
 * @return Generated random float
 */
inline float rand_float(float from, float to) {
    SGE_ASSERT(from < to);
    const float scale = rand() / (float) RAND_MAX;
    return from + scale * (to - from);
}

/**
 * @brief Generates a random boolean with a given probability of being true
 * 
 * @param probability The probability that the generated random boolean is true
 * @return Generated random boolean
 */
inline bool rand_bool(float probability) {
    if (probability >= 1.0f) return true;
    if (probability <= 0.0f) return false;

    return rand_float(0.0f, 1.0f) < probability;
}

/**
 * @brief Generates a random boolean
 * 
 * @return Generated random boolean
 */
inline bool rand_bool() {
    return rand() & 1;
}

}

_SGE_END

#endif