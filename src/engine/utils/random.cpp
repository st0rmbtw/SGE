#include <SGE/utils/random.hpp>
#include <random>

namespace {

struct RandomState {
    std::minstd_rand rng;
} state;

} // namespace


void sge::Random::Seed(std::uint32_t seed) noexcept {
    state.rng.seed(seed);
}

int sge::Random::Int(int from, int to) noexcept {
    std::uniform_int_distribution<int> distribution(from, to);
    return distribution(state.rng);
}

std::uint32_t sge::Random::UInt(std::uint32_t from, std::uint32_t to) noexcept {
    std::uniform_int_distribution<std::uint32_t> distribution(from, to);
    return distribution(state.rng);
}

float sge::Random::Float(float from, float to) noexcept {
    std::uniform_real_distribution<float> distribution(from, to);
    return distribution(state.rng);
}

double sge::Random::Double(double from, double to) noexcept {
    std::uniform_real_distribution<double> distribution(from, to);
    return distribution(state.rng);
}

bool sge::Random::Bool(float probability) noexcept {
    SGE_ASSERT(probability >= 0.0f && probability <= 1.0f);
    std::bernoulli_distribution distribution(probability);
    return distribution(state.rng);
}