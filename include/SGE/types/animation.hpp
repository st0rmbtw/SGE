#ifndef SGE_TYPES_ANIMATION_HPP_
#define SGE_TYPES_ANIMATION_HPP_

#include <SGE/time/stopwatch.hpp>

namespace sge {

enum class AnimationDirection : uint8_t {
    Forward = 0,
    Backward = 1
};

enum class RepeatStrategy : uint8_t {
    Repeat = 0,
    MirroredRepeat
};

class Animation {
public:
    Animation(sge::Duration::SecondsFloat duration, RepeatStrategy repeat_strategy) :
        m_duration{ duration },
        m_repeat{ repeat_strategy } {}
    
    void tick(float dt) {
        if (m_repeat == RepeatStrategy::MirroredRepeat) {
            const float delta = dt / (m_duration.count() * 0.5f);

            if (m_direction == AnimationDirection::Forward) {
                m_progress += delta;
            } else if (m_direction == AnimationDirection::Backward) {
                m_progress -= delta;
            }

            if (m_progress >= 1.0f) {
                m_progress = 1.0f;
                m_direction = AnimationDirection::Backward;
            } else if (m_progress <= 0.0f) {
                m_progress = 0.0f;
                m_direction = AnimationDirection::Forward;
            }
        } else {
            m_progress += dt / m_duration.count();

            if (m_progress >= 1.0f) {
                m_progress = 0.0f;
                m_direction = AnimationDirection::Forward;
            }
        }
    }

    void reset() {
        m_progress = 0.0f;
    }

    [[nodiscard]]
    inline float progress() const noexcept {
        return m_progress;
    }

    [[nodiscard]]
    inline AnimationDirection direction() const noexcept {
        return m_direction;
    }

private:
    sge::Duration::SecondsFloat m_duration;
    float m_progress = 0.0f;
    AnimationDirection m_direction = AnimationDirection::Forward;
    RepeatStrategy m_repeat = RepeatStrategy::Repeat;
};

}

#endif