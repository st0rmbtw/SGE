#ifndef SGE_TYPES_TRANSFORM_HPP_
#define SGE_TYPES_TRANSFORM_HPP_

#include <SGE/math/consts.hpp>
#include <SGE/math/quaternion.hpp>

namespace sge {

struct Transform {
    sge::Quaternion rotation;
    glm::vec3 translation = sge::consts::vec3::ZERO;
    glm::vec3 scale = sge::consts::vec3::ONE;

    inline static Transform FromTranslation(glm::vec3 translation) noexcept {
        Transform transform;
        transform.translation = translation;
        return transform;
    }

    inline static Transform FromRotation(Quaternion rotation) noexcept {
        Transform transform;
        transform.rotation = rotation;
        return transform;
    }

    inline static Transform FromScale(glm::vec3 scale) noexcept {
        Transform transform;
        transform.scale = scale;
        return transform;
    }

    inline static Transform FromXYZ(float x, float y, float z) noexcept {
        return FromTranslation(glm::vec3(x, y, z));
    }

    Transform& WithTranslation(glm::vec3 translation) {
        this->translation = translation;
        return *this;
    }

    Transform& WithRotation(sge::Quaternion rotation) {
        this->rotation = rotation;
        return *this;
    }

    Transform& WithScale(glm::vec3 scale) {
        this->scale = scale;
        return *this;
    }

    inline void Rotate(const Quaternion& rotation) noexcept {
        this->rotation = rotation * this->rotation;
    }

    inline void RotateAxis(const glm::vec3& axis, float angleRadians) noexcept {
        Rotate(Quaternion::FromRotation(axis, angleRadians));
    }

    inline void RotateX(float angleRadians) noexcept {
        Rotate(Quaternion::FromRotationX(angleRadians));
    }

    inline void RotateY(float angleRadians) noexcept {
        Rotate(Quaternion::FromRotationY(angleRadians));
    }

    inline void RotateZ(float angleRadians) noexcept {
        Rotate(Quaternion::FromRotationZ(angleRadians));
    }

    inline void RotateLocal(const Quaternion& rotation) noexcept {
        this->rotation *= rotation;
    }

    inline void RotateLocalAxis(const glm::vec3& axis, float angleRadians) noexcept {
        RotateLocal(Quaternion::FromRotation(axis, angleRadians));
    }

    inline void RotateLocalX(float angleRadians) noexcept {
        RotateLocal(Quaternion::FromRotationX(angleRadians));
    }

    inline void RotateLocalY(float angleRadians) noexcept {
        RotateLocal(Quaternion::FromRotationY(angleRadians));
    }

    inline void RotateLocalZ(float angleRadians) noexcept {
        RotateLocal(Quaternion::FromRotationZ(angleRadians));
    }

    inline void TranslateAround(glm::vec3 point, Quaternion rotation) noexcept {
        this->translation = point + rotation * (this->translation - point);
    }

    inline void RotateAround(glm::vec3 point, Quaternion rotation) {
        TranslateAround(point, rotation);
        Rotate(rotation);
    }

    inline void LookAt(glm::vec3 target, glm::vec3 up) {
        LookTo(glm::normalize(target - this->translation), up);
    }

    inline void LookTo(glm::vec3 direction, glm::vec3 up) {
        this->rotation = this->rotation.LookToRH(direction, up);
    }

    [[nodiscard]]
    inline glm::vec3 TransformPoint(glm::vec3 point) const noexcept {
        point = this->scale * point;
        point = this->rotation * point;
        point += this->translation;
        return point;
    }

    [[nodiscard]]
    inline glm::mat4 ComputeMatrix() const noexcept {
        glm::mat3 matrix = glm::mat3_cast(glm::quat(this->rotation));
        return glm::mat4(
            glm::vec4(matrix[0] * this->scale.x, 0.f),
            glm::vec4(matrix[1] * this->scale.y, 0.f),
            glm::vec4(matrix[2] * this->scale.z, 0.f),
            glm::vec4(translation, 1.f)
        );
    }

    [[nodiscard]]
    inline glm::vec3 GetLocalX() const noexcept {
        return this->rotation * sge::consts::vec3::X;
    }

    [[nodiscard]]
    inline glm::vec3 GetLocalY() const noexcept {
        return this->rotation * sge::consts::vec3::Y;
    }

    [[nodiscard]]
    inline glm::vec3 GetLocalZ() const noexcept {
        return this->rotation * sge::consts::vec3::Z;
    }

    [[nodiscard]]
    inline glm::vec3 Left() const noexcept {
        return -GetLocalX();
    }

    [[nodiscard]]
    inline glm::vec3 Right() const noexcept {
        return GetLocalX();
    }

    [[nodiscard]]
    inline glm::vec3 Up() const noexcept {
        return GetLocalY();
    }

    [[nodiscard]]
    inline glm::vec3 Down() const noexcept {
        return -GetLocalY();
    }

    [[nodiscard]]
    inline glm::vec3 Forward() const noexcept {
        return -GetLocalZ();
    }

    [[nodiscard]]
    inline glm::vec3 Back() const noexcept {
        return GetLocalZ();
    }
};

} // namespace sge

#endif // SGE_TYPES_TRANSFORM_HPP_