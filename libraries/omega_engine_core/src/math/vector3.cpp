#include <omega/core/math/vector3.hpp>

#include <glm/geometric.hpp>

namespace omega
{

Vector3 normalize(const Vector3& vec)
{
    return glm::normalize(vec);
}

float dot(const Vector3& x, const Vector3& y)
{
    return glm::dot(x, y);
}

} // namespace omega