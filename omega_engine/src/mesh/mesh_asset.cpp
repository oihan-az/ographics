#include <omega/mesh/mesh_asset.hpp>

#include <algorithm>

namespace
{

uint32_t format_size(ogfx::Format format)
{
    switch (format)
    {
    case ogfx::Format::R8_UNORM:
        return 1;
    case ogfx::Format::RG8_UNORM:
        return 2;
    case ogfx::Format::RGBA8_UNORM:
        return 4;
    case ogfx::Format::BGRA8_UNORM:
        return 4;
    case ogfx::Format::RGBA8_SRGB:
        return 4;
    case ogfx::Format::BGRA8_SRGB:
        return 4;

    case ogfx::Format::RG16_FLOAT:
        return 4;
    case ogfx::Format::RGBA16_FLOAT:
        return 8;

    case ogfx::Format::RG32_FLOAT:
        return 8;
    case ogfx::Format::RGB32_FLOAT:
        return 12;
    case ogfx::Format::RGBA32_FLOAT:
        return 16;

    case ogfx::Format::Undefined:
    case ogfx::Format::D16_UNORM:
    case ogfx::Format::D24_UNORM_S8_UINT:
    case ogfx::Format::D32_FLOAT:
    case ogfx::Format::D32_FLOAT_S8_UINT:
        return 0;
    }

    return 0;
}

}

namespace omega
{

bool validate_mesh_asset(const MeshAsset& asset)
{
    const auto& bindings = asset.vertex_input.bindings;

    if (asset.vertex_count == 0)
        return false;

    // Require one vertex stream per binding
    if (asset.vertex_data.size() != bindings.size())
        return false;

    // Check duplicate bindings
    for (std::size_t i = 0; i < bindings.size(); ++i)
    {
        for (std::size_t j = i + 1; j < bindings.size(); ++j)
        {
            if (bindings[i].binding == bindings[j].binding)
                return false;
        }
    }

    // Validate each binding and its corresponding stream
    for (std::size_t i = 0; i < bindings.size(); ++i)
    {
        const auto& binding = bindings[i];
        const auto& data = asset.vertex_data[i];

        if (binding.input_rate != ogfx::VertexInputRate::Vertex)
            return false;

        if (binding.stride == 0)
            return false;

        const std::size_t expected_size = static_cast<std::size_t>(asset.vertex_count) * binding.stride;

        if (data.size() != expected_size)
            return false;
    }

    // Validate vertex attribute references
    for (const auto& attribute : asset.vertex_input.attributes)
    {
        const uint32_t size = format_size(attribute.format);

        if (size == 0)
            return false;

        auto binding = std::find_if(bindings.begin(), bindings.end(), [&](const ogfx::VertexBindingDesc& desc)
                                    { return desc.binding == attribute.binding; });

        if (binding == bindings.end())
            return false;

        if (attribute.offset > binding->stride || size > binding->stride - attribute.offset)
            return false;
    }

    // Validate indices
    for (uint32_t index : asset.indices)
    {
        if (index >= asset.vertex_count)
            return false;
    }

    return true;
}

} // namespace omega