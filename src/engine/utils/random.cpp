#include <SGE/utils/random.hpp>
#include <random>

namespace {

uint64_t splitmix64(uint64_t& state) {
    uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

uint64_t xoshiro256pp_rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

class Xoshiro256pp {
public:
    using result_type = uint64_t;
public:
    Xoshiro256pp() = default;

    explicit Xoshiro256pp(std::uint64_t s) {
        seed(s);
    }

    void seed(std::uint64_t seed) {
        m_state[0] = splitmix64(seed);
        m_state[1] = splitmix64(seed);
        m_state[2] = splitmix64(seed);
        m_state[3] = splitmix64(seed);
    }

    result_type operator()() {
        const std::uint64_t result = xoshiro256pp_rotl(m_state[0] + m_state[3], 23) + m_state[0];
        const std::uint64_t t = m_state[1] << 17;
        m_state[2] ^= m_state[0];
        m_state[3] ^= m_state[1];
        m_state[1] ^= m_state[2];
        m_state[0] ^= m_state[3];
        m_state[2] ^= t;
        m_state[3] = xoshiro256pp_rotl(m_state[3], 45);
        return result;
    }

    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

private:
    std::uint64_t m_state[4];
};

template <typename Engine>
struct BoolStream {
    Engine::result_type bits = 0;
    int remaining = 0;

    bool next(Xoshiro256pp& rng) {
        if (remaining == 0) {
            bits = rng();
            remaining = std::numeric_limits<typename Engine::result_type>::digits;
        }
        return (bits >> --remaining) & 1;
    }
};

struct RandomState {
    Xoshiro256pp rng;
    BoolStream<Xoshiro256pp> bool_stream;
} state;

float uniform_float() {
    uint32_t x = state.rng() >> 32;
    return (x >> 9) * 0x1.0p-23f;
}

} // namespace

void sge::Random::Seed(std::uint64_t seed) noexcept {
    state.rng.seed(seed);
}

int sge::Random::Int(std::uniform_int_distribution<int>& distribution) noexcept {
    return distribution(state.rng);
}

std::uint32_t sge::Random::UInt(std::uniform_int_distribution<std::uint32_t>& distribution) noexcept {
    return distribution(state.rng);
}

std::uint32_t sge::Random::UInt(std::uint32_t from, std::uint32_t to) noexcept {
    const uint32_t range = to - from + 1;
    uint32_t x = state.rng();
    uint64_t m = uint64_t(x) * uint64_t(range);
    uint32_t l = uint32_t(m);
    if (l < range) {
        uint32_t t = (-range) % range;
        while (l < t) {
            x = state.rng();
            m = uint64_t(x) * uint64_t(range);
            l = uint32_t(m);
        }
    }
    return from + (m >> 32);
}

float sge::Random::Float(float from, float to) noexcept {
    const float x = uniform_float();
    return from + x * (to - from);
}

double sge::Random::Double(double from, double to) noexcept {
    uint64_t x = state.rng();
    double m = (x >> 12) * 0x1.0p-52;
    return from + m * (to - from);
}

bool sge::Random::Bool(float probability) noexcept {
    return uniform_float() < probability;
}

bool sge::Random::Bool() noexcept {
    return state.bool_stream.next(state.rng);
}