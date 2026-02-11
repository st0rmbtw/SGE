#include <SGE/time/time.hpp>

using namespace sge;

static struct TimeState {
    delta_time_t delta = delta_time_t(0.0);
    delta_time_t fixed_delta = delta_time_t(1.0 / 60.0);
    float elapsed_seconds = 0.0;
    float fixed_elapsed_seconds = 0.0;
} state;

void Time::SetFixedTimestepSeconds(const float seconds) noexcept {
    state.fixed_delta = delta_time_t(seconds);
}

void Time::AdvanceBy(const delta_time_t& delta) noexcept {
    state.delta = delta;
    state.elapsed_seconds += delta.count();
}

void Time::AdvanceFixed() noexcept {
    state.fixed_elapsed_seconds += state.fixed_delta.count();
}

const delta_time_t& Time::Delta() noexcept {
    return state.delta;
}

float Time::DeltaSeconds() noexcept {
    return state.delta.count();
}

float Time::ElapsedSeconds() noexcept {
    return state.elapsed_seconds;
}

const delta_time_t Time::FixedDelta() noexcept {
    return state.fixed_delta;
}

float Time::FixedDeltaSeconds() noexcept {
    return state.fixed_delta.count();
}

float Time::FixedElapsedSeconds() noexcept {
    return state.fixed_elapsed_seconds;
}