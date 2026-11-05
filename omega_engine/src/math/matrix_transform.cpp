#include <omega/math/matrix_transform.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace omega
{

Matrix4 translate_mtx(const Vector3& translation)
{
    return glm::translate(Matrix4(1.0f), translation);
}

Matrix4 rotate_mtx(const Quaternion& rotation)
{
    return glm::mat4_cast(rotation);
}

Matrix4 scale_mtx(const Vector3& scale)
{
    return glm::scale(Matrix4(1.0f), scale);
}

} // namespace omega