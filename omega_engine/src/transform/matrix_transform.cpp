#include <omega/transform/transform.hpp>

#include <omega/core/math/matrix_transform.hpp>

namespace omega
{

Matrix4 mtx_transform(const Transform& tr)
{
    return mtx_translate(tr.position) * mtx_rotate(tr.rotation) * mtx_scale(tr.scale);
}

} // namespace omega