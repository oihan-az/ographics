#include <omega/core/math/matrix_transform.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace omega
{

Matrix4 mtx_translate(const Vector3& translation)
{
    return glm::translate(Matrix4(1.0f), translation);
}

Matrix4 mtx_rotate(const Quaternion& rotation)
{
    return glm::mat4_cast(rotation);
}

Matrix4 mtx_scale(const Vector3& scale)
{
    return glm::scale(Matrix4(1.0f), scale);
}

} // namespace omega