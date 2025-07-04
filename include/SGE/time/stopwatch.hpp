#ifndef _SGE_TIME_STOPWATCH_HPP_
#define _SGE_TIME_STOPWATCH_HPP_

#pragma once

#include <chrono>

#include "../defines.hpp"

_SGE_BEGIN

namespace Duration {
    using Nanos = std::chrono::duration<uint64_t, std::nano>;
    using Millis = std::chrono::duration<uint64_t, std::milli>;
    using Seconds = std::chrono::duration<uint64_t>;
    using SecondsFloat = std::chrono::duration<float>;
    using Minutes = std::chrono::duration<uint32_t, std::ratio<60>>;
    using Hours = std::chrono::duration<uint32_t, std::ratio<3600>>;

    template <class _To, class _Rep, class _Period>
    constexpr inline _To Cast(const std::chrono::duration<_Rep, _Period>& _Dur) {
        return std::chrono::duration_cast<_To>(_Dur);
    }
};

class Stopwatch {
public:
    using duration_t = Duration::Nanos;

    Stopwatch() : m_elapsed(duration_t::zero()) {}

    [[nodiscard]] inline duration_t elapsed() const { return m_elapsed; }
    [[nodiscard]] inline bool paused() const { return m_paused; }
    [[nodiscard]] inline float elapsed_secs() const { return Duration::Cast<Duration::Seconds>(m_elapsed).count(); }

    inline void reset() { m_elapsed = duration_t::zero(); }
    inline void pause() { m_paused = true; }
    inline void unpause() { m_paused = false; }

    template <class Rep, class Period>
    inline void set_elapsed(const std::chrono::duration<Rep, Period>& elapsed) {
        m_elapsed = Duration::Cast<duration_t>(elapsed);
    }

    template <class Rep, class Period>
    inline void tick(const std::chrono::duration<Rep, Period>& delta) {
        if (!m_paused) {
            duration_t d = Duration::Cast<duration_t>(delta);
            m_elapsed += d;
        }
    }

private:
    duration_t m_elapsed;
    bool m_paused = false;
};

_SGE_END

#endif