#pragma once

#include <omega/core/math/vector3.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/texture/texture_handle.hpp>

namespace omega
{

struct PbrOpaqueMaterial
{
    // Base color
    Vector4 base_color_factor{1.0f};

    TextureHandle base_color_texture{};

    // Metallic-roughness workflow
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;

    TextureHandle metallic_roughness_texture{};

    // Normal mapping
    TextureHandle normal_texture{};
    float normal_scale = 1.0f;

    // Ambient occlusion
    TextureHandle occlusion_texture{};
    float occlusion_strength = 1.0f;

    // Emissive
    Vector3 emissive_factor{0.0f};
    TextureHandle emissive_texture{};
};

} // namespace omega