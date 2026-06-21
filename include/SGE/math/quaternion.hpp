#pragma once

#ifndef _SGE_MATH_QUAT_HPP_
#define _SGE_MATH_QUAT_HPP_

#include <SGE/math/consts.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace sge {

class Quaternion {
public:
    Quaternion() = default;

    explicit Quaternion(glm::quat quat) : m_quat(quat) {}

    static Quaternion FromEuler(float yawRadians, float pitchRadians, float rollRadians) {
        return Quaternion(glm::quat(glm::vec3(pitchRadians, yawRadians, rollRadians)));
    }

    static Quaternion FromRotation(const glm::vec3& axis, float angleRadians) {
        return Quaternion(glm::angleAxis(angleRadians, axis));
    }

    static Quaternion FromRotationX(float angleRadians) {
        return Quaternion(glm::angleAxis(angleRadians, sge::consts::vec3::X));
    }

    static Quaternion FromRotationY(float angleRadians) {
        return Quaternion(glm::angleAxis(angleRadians, sge::consts::vec3::Y));
    }

    static Quaternion FromRotationZ(float angleRadians) {
        return Quaternion(glm::angleAxis(angleRadians, sge::consts::vec3::Z));
    }

    [[nodiscard]]
    inline Quaternion Inverse() const noexcept {
        return Quaternion(glm::inverse(m_quat));
    }

    [[nodiscard]]
    inline Quaternion Conjugate() const noexcept {
        return Quaternion(glm::conjugate(m_quat));
    }

    [[nodiscard]]
    inline Quaternion Normalize() const noexcept {
        return Quaternion(glm::normalize(m_quat));
    }

    [[nodiscard]]
    float GetYaw() const noexcept {
        return glm::yaw(m_quat);
    }

    [[nodiscard]]
    float GetPitch() const noexcept {
        return glm::pitch(m_quat);
    }

    [[nodiscard]]
    float GetRoll() const noexcept {
        return glm::roll(m_quat);
    }

    // YXZ plane
    [[nodiscard]]
    inline glm::vec3 ToEuler() const noexcept {
        return glm::eulerAngles(m_quat);
    }

    [[nodiscard]]
    inline Quaternion LookToLH(glm::vec3 direction, glm::vec3 up) const noexcept {
        return Quaternion(glm::quatLookAtLH(direction, up));
    }

    [[nodiscard]]
    inline Quaternion LookToRH(glm::vec3 direction, glm::vec3 up) const noexcept {
        return Quaternion(glm::quatLookAtRH(direction, up));
    }

    inline GLM_CONSTEXPR Quaternion operator*(const float scalar) const noexcept {
        return Quaternion(m_quat * scalar);
    }

    inline GLM_CONSTEXPR Quaternion operator/(const float scalar) const noexcept {
        return Quaternion(m_quat / scalar);
    }

    inline GLM_CONSTEXPR Quaternion operator*(const Quaternion& other) const noexcept {
        return Quaternion(m_quat * other.m_quat);
    }

    inline GLM_CONSTEXPR Quaternion operator+(const Quaternion& other) const noexcept {
        return Quaternion(m_quat + other.m_quat);
    }

    inline GLM_CONSTEXPR Quaternion operator-(const Quaternion& other) const noexcept {
        return Quaternion(m_quat - other.m_quat);
    }

    inline GLM_CONSTEXPR Quaternion operator*=(const float scalar) noexcept {
        m_quat *= scalar;
        return *this;
    }

    inline GLM_CONSTEXPR Quaternion operator/=(const float scalar) noexcept {
        m_quat /= scalar;
        return *this;
    }

    inline GLM_CONSTEXPR Quaternion operator*=(const Quaternion& other) noexcept {
        m_quat *= other.m_quat;
        return *this;
    }

    inline GLM_CONSTEXPR Quaternion& operator+=(const Quaternion& other) noexcept {
        m_quat += other.m_quat;
        return *this;
    }

    inline GLM_CONSTEXPR Quaternion& operator-=(const Quaternion& other) noexcept {
        m_quat -= other.m_quat;
        return *this;
    }

	inline GLM_CONSTEXPR bool operator==(const Quaternion& q) const noexcept {
		return m_quat == q.m_quat;
	}

    explicit operator glm::quat() const noexcept {
        return m_quat;
    }

private:
    glm::quat m_quat = glm::identity<glm::quat>();
};

inline GLM_CONSTEXPR glm::vec3 operator*(const Quaternion& quat, glm::vec3 v) noexcept {
    return glm::quat(quat) * v;
}

inline GLM_CONSTEXPR glm::vec4 operator*(const Quaternion& quat, glm::vec4 v) noexcept {
    return glm::quat(quat) * v;
}

} // namespace sge

#endif