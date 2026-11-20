#include <omega/core/math/matrix4.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/matrix.hpp>

namespace omega
{

Matrix4 inverse(const Matrix4& matrix)
{
    return glm::inverse(matrix);
}

Matrix4 transpose(const Matrix4& matrix)
{
    return glm::transpose(matrix);
}

float determinant(const Matrix4& matrix)
{
    return glm::determinant(matrix);
}

bool decompose(const Matrix4& matrix, Vector3& scale, Quaternion& rotation, Vector3& position,
               Vector3& skew, Vector4& perspective)
{
    return glm::decompose(matrix, scale, rotation, position, skew, perspective);
}

} // namespace omega