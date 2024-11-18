#ifndef TIME_TIME_HPP
#define TIME_TIME_HPP

#pragma once

#include <chrono>

using delta_time_t = std::chrono::duration<float>;

namespace Time {
    const delta_time_t& delta();
    float delta_seconds();
    float elapsed_seconds();
    const delta_time_t& fixed_delta();
    float fixed_delta_seconds();
    float fixed_elapsed_seconds();

    void set_fixed_delta(const float seconds);

    template <class Rep, class Period>
    void set_fixed_delta(const std::chrono::duration<Rep, Period>& delta) {
        const delta_time_t d = std::chrono::duration_cast<delta_time_t>(delta);
        set_fixed_delta(d.count());
    }

    void advance_by(const delta_time_t& delta);
    void fixed_advance();
};

#endif