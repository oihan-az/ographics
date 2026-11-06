#include <omega/transform/transform.hpp>
#include <omega/core/math/matrix_transform.hpp>

namespace omega
{

Matrix4 Transform::matrix() const
{
    return translate_mtx(position) * rotate_mtx(rotation) * scale_mtx(scale);
}

} // namespace omega