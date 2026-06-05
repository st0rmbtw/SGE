#ifndef _SGE_UTILS_RANDOM_HPP_
#define _SGE_UTILS_RANDOM_HPP_

#include <SGE/assert.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <random>

namespace sge {

class RandomEngine {
public:
    using result_type = std::uint64_t;
public:
    RandomEngine() = default;
    
    explicit RandomEngine(std::uint64_t s) noexcept {
        Seed(s);
    }

    void Seed(std::uint64_t value) noexcept;

    result_type operator()() noexcept;

    inline result_type Next() noexcept {
        return operator()();
    }

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }
private:
    std::uint64_t m_state[4];
};

namespace Random {

    RandomEngine& GetEngine() noexcept;

    /**
     * @brief Seeds the random generator.
     * 
     * @param seed The seed.
     */
    inline void Seed(std::uint64_t seed) noexcept {
        GetEngine().Seed(seed);
    }

    /**
     * @brief Returns the next generated raw pseude-random value.
     */
    inline std::uint64_t NextRaw() noexcept {
        return GetEngine().Next();
    }

    /**
    * @brief Generates a random integer using specified distribution.
    * 
    * @param distribution The distribution.
    * @return Generated random integer.
    */
    inline int Int(std::uniform_int_distribution<int>& distribution) noexcept {
        return distribution(GetEngine());
    }

    /**
    * @brief Generates a random integer within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random integer.
    */
    int Int(int from, int to) noexcept;

    /**
    * @brief Generates a random unsigned integer using specified distribution.
    * 
    * @param distribution The distribution.
    * @return Generated unsigned random integer.
    */
    inline std::uint32_t UInt(std::uniform_int_distribution<std::uint32_t>& distribution) noexcept {
        return distribution(GetEngine());
    }

    /**
    * @brief Generates a random unsigned integer within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random unsigned integer.
    */
    std::uint32_t UInt(std::uint32_t from, std::uint32_t to) noexcept;

    /**
    * @brief Generates a random float using specified distribution.
    * 
    * @param distribution The distribution.
    * @return Generated random float.
    */
    inline float Float(std::uniform_real_distribution<float>& distribution) noexcept {
        return distribution(GetEngine());
    }

    /**
    * @brief Generates a random float within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random float.
    */
    float Float(float from, float to) noexcept;

    /**
    * @brief Generates a random double using specified distribution.
    * 
    * @param distribution The distribution.
    * @return Generated random double.
    */
    inline double Double(std::uniform_real_distribution<double>& distribution) noexcept {
        return distribution(GetEngine());
    }

    /**
    * @brief Generates a random double within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random double.
    */
    double Double(double from, double to) noexcept;

    /**
    * @brief Generates a random boolean with a given probability of being true.
    * 
    * @param probability The probability that the generated random boolean is true.
    * @return Generated random boolean.
    */
    bool Bool(float probability) noexcept;

    /**
    * @brief Generates a random boolean.
    * 
    * @return Generated random boolean.
    */
    bool Bool() noexcept;

    /**
    * @brief Generates a random glm::vec2 within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random glm::vec2.
    */
    inline glm::vec2 Vec2(glm::vec2 from, glm::vec2 to) noexcept {
        return glm::vec2(Float(from.x, to.x), Float(from.y, to.y));
    }

    /**
    * @brief Generates a random glm::uvec2 within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random glm::uvec2.
    */
    inline glm::uvec2 UVec2(glm::uvec2 from, glm::uvec2 to) noexcept {
        return glm::uvec2(UInt(from.x, to.x), UInt(from.y, to.y));
    }

    /**
    * @brief Generates a random glm::ivec2 within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random glm::ivec2.
    */
    inline glm::ivec2 IVec2(glm::ivec2 from, glm::ivec2 to) noexcept {
        return glm::ivec2(Int(from.x, to.x), Int(from.y, to.y));
    }

    /**
    * @brief Generates a random glm::vec3 within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random glm::vec3.
    */
    inline glm::vec3 Vec3(glm::vec3 from, glm::vec3 to) noexcept {
        return glm::vec3(Float(from.x, to.x), Float(from.y, to.y), Float(from.z, to.z));
    }

    /**
    * @brief Generates a random glm::uvec3 within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random glm::uvec3.
    */
    inline glm::uvec3 UVec3(glm::uvec3 from, glm::uvec3 to) noexcept {
        return glm::uvec3(UInt(from.x, to.x), UInt(from.y, to.y), UInt(from.z, to.z));
    }

    /**
    * @brief Generates a random glm::ivec3 within a given range.
    * 
    * @param from Specifies the start of the range (inclusive).
    * @param to Specifies the end of the range (inclusive).
    * @return Generated random glm::ivec3.
    */
    inline glm::ivec3 IVec3(glm::ivec3 from, glm::ivec3 to) noexcept {
        return glm::ivec3(Int(from.x, to.x), Int(from.y, to.y), Int(from.z, to.z));
    }

} // namespace Random


} // namespace sge



#endif