#include <SGE/time/time.hpp>

using namespace sge;

static struct TimeState {
    delta_time_t delta = delta_time_t(0.0);
    delta_time_t fixed_delta = delta_time_t(1.0 / 60.0);
    double elapsed_seconds = 0.0;
    double fixed_elapsed_seconds = 0.0;
    double overstep = 0.0;
} state;

void Time::SetFixedTimestepSeconds(const double seconds) noexcept {
    state.fixed_delta = delta_time_t(seconds);
}

void Time::AdvanceBy(const delta_time_t& delta) noexcept {
    state.delta = delta;
    state.elapsed_seconds += delta.count();
    state.overstep += delta.count();
}

void Time::AdvanceFixed() noexcept {
    state.fixed_elapsed_seconds += state.fixed_delta.count();
    state.overstep -= state.fixed_delta.count();
}

const delta_time_t& Time::Delta() noexcept {
    return state.delta;
}

double Time::DeltaSeconds() noexcept {
    return state.delta.count();
}

double Time::ElapsedSeconds() noexcept {
    return state.elapsed_seconds;
}

const delta_time_t Time::FixedDelta() noexcept {
    return state.fixed_delta;
}

double Time::FixedDeltaSeconds() noexcept {
    return state.fixed_delta.count();
}

double Time::FixedElapsedSeconds() noexcept {
    return state.fixed_elapsed_seconds;
}

double Time::Overstep() noexcept {
    return state.overstep;
}

double Time::OverstepFraction() noexcept {
    return state.overstep / state.fixed_delta.count();
}