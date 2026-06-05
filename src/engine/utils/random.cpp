#include <SGE/utils/random.hpp>

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

template <typename Engine>
struct BoolStream {
    Engine::result_type bits = 0;
    int remaining = 0;

    bool next(Engine& rng) {
        if (remaining == 0) {
            bits = rng();
            remaining = std::numeric_limits<typename Engine::result_type>::digits;
        }
        return (bits >> --remaining) & 1;
    }
};

struct RandomState {
    sge::RandomEngine rng;
    BoolStream<sge::RandomEngine> bool_stream;
} state;

float uniform_float() {
    uint32_t x = state.rng() >> 32;
    return (x >> 9) * 0x1.0p-23f;
}

std::uint32_t uniform_uint32(std::uint32_t range) {
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
    return m >> 32;
}

} // namespace

void sge::RandomEngine::Seed(std::uint64_t seed) noexcept {
    m_state[0] = splitmix64(seed);
    m_state[1] = splitmix64(seed);
    m_state[2] = splitmix64(seed);
    m_state[3] = splitmix64(seed);
}

// Xoshiro256++
sge::RandomEngine::result_type sge::RandomEngine::operator()() noexcept {
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

sge::RandomEngine& sge::Random::GetEngine() noexcept {
    return state.rng;
}

int sge::Random::Int(int from, int to) noexcept {
    return from + (int)uniform_uint32(std::uint32_t(to - from + 1));
}

std::uint32_t sge::Random::UInt(std::uint32_t from, std::uint32_t to) noexcept {
    return from + uniform_uint32(to - from + 1);
}

float sge::Random::Float(float from, float to) noexcept {
    const float x = uniform_float();
    return from + x * (to - from);
}

double sge::Random::Double(double from, double to) noexcept {
    std::uint64_t x = state.rng();
    double m = (x >> 12) * 0x1.0p-52;
    return from + m * (to - from);
}

bool sge::Random::Bool(float probability) noexcept {
    return uniform_float() < probability;
}

bool sge::Random::Bool() noexcept {
    return state.bool_stream.next(state.rng);
}