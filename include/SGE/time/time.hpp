#ifndef _SGE_TIME_TIME_HPP_
#define _SGE_TIME_TIME_HPP_

#pragma once

#include <chrono>

namespace sge {

using delta_time_t = std::chrono::duration<float>;

namespace Time {
    const delta_time_t& Delta() noexcept;
    const delta_time_t FixedDelta() noexcept;
    float DeltaSeconds() noexcept;
    float ElapsedSeconds() noexcept;
    float FixedDeltaSeconds() noexcept;
    float FixedElapsedSeconds() noexcept;

    void SetFixedTimestepSeconds(const float seconds) noexcept;

    template <class Rep, class Period>
    void SetFixedTimestep(const std::chrono::duration<Rep, Period>& delta) {
        const delta_time_t d = std::chrono::duration_cast<delta_time_t>(delta);
        SetFixedTimestepSeconds(d.count());
    }
    
    void AdvanceBy(const delta_time_t& delta) noexcept;
    void AdvanceFixed() noexcept;
};

}

#endif