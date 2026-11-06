#include <omega/core/math/matrix4.hpp>

#include <glm/gtc/matrix_inverse.hpp>
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

} // namespace omega