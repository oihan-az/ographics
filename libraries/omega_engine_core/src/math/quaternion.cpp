#include <omega/core/math/quaternion.hpp>

namespace omega
{

Quaternion angle_axis(float angle, const Vector3& axis)
{
    return glm::angleAxis(angle, axis);
}

} // namespace omega