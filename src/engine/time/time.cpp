#include <SGE/time/time.hpp>

using namespace sge;

static struct TimeState {
    delta_time_t delta;
    delta_time_t fixed_delta;
    float elapsed_seconds;
    float fixed_elapsed_seconds;
} state;

void Time::SetFixedTimestepSeconds(const float seconds) {
    state.fixed_delta = delta_time_t(seconds);
}

void Time::AdvanceBy(const delta_time_t& delta) {
    state.delta = delta;
    state.elapsed_seconds += delta.count();
}

void Time::AdvanceFixed() {
    state.fixed_elapsed_seconds += state.fixed_delta.count();
}

const delta_time_t& Time::Delta() {
    return state.delta;
}

float Time::DeltaSeconds() {
    return state.delta.count();
}

float Time::ElapsedSeconds() {
    return state.elapsed_seconds;
}

const delta_time_t Time::FixedDelta() {
    return state.fixed_delta;
}

float Time::FixedDeltaSeconds() {
    return state.fixed_delta.count();
}

float Time::FixedElapsedSeconds() {
    return state.fixed_elapsed_seconds;
}