#ifndef _SGE_TIME_TIME_HPP_
#define _SGE_TIME_TIME_HPP_

#pragma once

#include <chrono>

#include "../defines.hpp"

_SGE_BEGIN

using delta_time_t = std::chrono::duration<float>;

namespace Time {
    const delta_time_t& Delta();
    const delta_time_t FixedDelta();
    float DeltaSeconds();
    float ElapsedSeconds();
    float FixedDeltaSeconds();
    float FixedElapsedSeconds();

    void SetFixedTimestepSeconds(const float seconds);

    template <class Rep, class Period>
    void SetFixedTimestep(const std::chrono::duration<Rep, Period>& delta) {
        const delta_time_t d = std::chrono::duration_cast<delta_time_t>(delta);
        SetFixedTimestepSeconds(d.count());
    }
    
    void AdvanceBy(const delta_time_t& delta);
    void AdvanceFixed();
};

_SGE_END

#endif