#include <SGE/time/time.hpp>

using namespace sge::time;

static struct TimeState {
    delta_time_t delta;
    delta_time_t fixed_delta;
    float elapsed_seconds;
    float fixed_elapsed_seconds;
} state;

const delta_time_t& Time::Delta(void) { return state.delta; }
float Time::DeltaSeconds(void) { return state.delta.count(); }
float Time::ElapsedSeconds(void) { return state.elapsed_seconds; }
const delta_time_t Time::FixedDelta(void) { return state.fixed_delta; }
float Time::FixedDeltaSeconds(void) { return state.fixed_delta.count(); }
float Time::FixedElapsedSeconds(void) { return state.fixed_elapsed_seconds; }

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