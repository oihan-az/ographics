#include <omega/core/math/quaternion.hpp>

namespace omega
{

Quaternion angle_axis(float angle, const Vector3& axis)
{
    return glm::angleAxis(angle, axis);
}

Vector3 euler_angles(const Quaternion& quat)
{
    return glm::eulerAngles(quat);
}

} // namespace omega