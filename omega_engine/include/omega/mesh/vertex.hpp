#ifndef OMEGA_VERTEX_HPP
#define OMEGA_VERTEX_HPP

#include <omega/core/math/vector2.hpp>
#include <omega/core/math/vector3.hpp>

#include <ogfx/vertex_input.hpp>

namespace omega
{

struct Vertex
{
    Vector3 position{0.0f};
    Vector2 uv{0.0f};
    Vector3 normal{0.0f};
    Vector3 tangent{0.0f};
};

ogfx::VertexInputStateDesc standard_vertex_layout();

} // namespace omega

#endif
