#include <omega/core/math/vector3.hpp>

#include <glm/geometric.hpp>

namespace omega
{

Vector3 normalize(const Vector3& vec)
{
    return glm::normalize(vec);
}

} // namespace omega