#include <omega/core/math/vector4.hpp>

#include <glm/geometric.hpp>

namespace omega
{

Vector4 normalize(const Vector4& vec)
{
    return glm::normalize(vec);
}

} // namespace omega