#ifndef OMEGA_VERTEX_LAYOUT_HPP
#define OMEGA_VERTEX_LAYOUT_HPP

#include <ogfx/vertex_input.hpp>

namespace omega
{

struct VertexLayout
{
    ogfx::VertexInputStateDesc desc;

    bool operator==(const VertexLayout& other) const
    {
        if (desc.bindings.size() != other.desc.bindings.size() ||
            desc.attributes.size() != other.desc.attributes.size())
            return false;

        for (std::size_t i = 0; i < desc.bindings.size(); ++i)
        {
            const auto& a = desc.bindings[i];
            const auto& b = other.desc.bindings[i];

            if (a.binding != b.binding || a.stride != b.stride || a.input_rate != b.input_rate)
                return false;
        }

        for (std::size_t i = 0; i < desc.attributes.size(); ++i)
        {
            const auto& a = desc.attributes[i];
            const auto& b = other.desc.attributes[i];

            if (a.location != b.location || a.binding != b.binding || a.format != b.format || a.offset != b.offset)
                return false;
        }

        return true;
    }
};

struct VertexLayoutId
{
    static constexpr uint32_t invalid_index = std::numeric_limits<uint32_t>::max();
    uint32_t value = invalid_index;

    constexpr bool is_valid() const { return value != invalid_index; }
    constexpr bool operator==(const VertexLayoutId&) const = default;
};

} // namespace omega

#endif // OMEGA_VERTEX_LAYOUT_HPP