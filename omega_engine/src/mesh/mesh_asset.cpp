#include <omega/mesh/mesh_asset.hpp>

#include <algorithm>

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
        ogfx::FormatInfo format_info;
        bool result = ogfx::get_format_info(attribute.format, format_info);
        if (!result)
            return false;

        uint32_t size = format_info.bytes_per_pixel;

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