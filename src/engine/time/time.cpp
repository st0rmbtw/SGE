#include <SGE/time/time.hpp>

namespace {

struct TimeState {
    sge::delta_time_t delta = sge::delta_time_t(0.0);
    sge::delta_time_t fixed_delta = sge::delta_time_t(1.0 / 60.0);
    double elapsed_seconds = 0.0;
    double fixed_elapsed_seconds = 0.0;
    double overstep = 0.0;
} state;

} // namespace

void sge::Time::SetFixedTimestepSeconds(const double seconds) noexcept {
    state.fixed_delta = delta_time_t(seconds);
}

void sge::Time::AdvanceBy(const delta_time_t& delta) noexcept {
    state.delta = delta;
    state.elapsed_seconds += delta.count();
    state.overstep += delta.count();
}

void sge::Time::AdvanceFixed() noexcept {
    state.fixed_elapsed_seconds += state.fixed_delta.count();
    state.overstep -= state.fixed_delta.count();
}

const sge::delta_time_t& sge::Time::Delta() noexcept {
    return state.delta;
}

double sge::Time::DeltaSeconds() noexcept {
    return state.delta.count();
}

double sge::Time::ElapsedSeconds() noexcept {
    return state.elapsed_seconds;
}

const sge::delta_time_t sge::Time::FixedDelta() noexcept {
    return state.fixed_delta;
}

double sge::Time::FixedDeltaSeconds() noexcept {
    return state.fixed_delta.count();
}

double sge::Time::FixedElapsedSeconds() noexcept {
    return state.fixed_elapsed_seconds;
}

double sge::Time::Overstep() noexcept {
    return state.overstep;
}

double sge::Time::OverstepFraction() noexcept {
    return state.overstep / state.fixed_delta.count();
}