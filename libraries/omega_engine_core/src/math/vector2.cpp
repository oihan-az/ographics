#include <omega/core/math/vector2.hpp>

#include <glm/geometric.hpp>

namespace omega
{

Vector2 normalize(const Vector2& vec)
{
    return glm::normalize(vec);
}

float dot(const Vector2& x, const Vector2& y)
{
    return glm::dot(x, y);
}

} // namespace omega