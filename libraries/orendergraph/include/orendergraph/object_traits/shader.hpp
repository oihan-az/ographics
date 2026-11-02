#ifndef ORENDERGRAPH_OBJECT_TRAITS_SHADER_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_SHADER_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/shader.hpp>

#include <memory>

namespace orendergraph
{

template <> struct ObjectTraits<ogfx::Shader>
{
    using DescType = ogfx::ShaderDesc;

    static std::unique_ptr<ogfx::Shader> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                const DescType& desc)
    {
        return std::make_unique<ogfx::Shader>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        return lhs.stage == rhs.stage &&
               lhs.entry_point == rhs.entry_point &&
               lhs.bytecode == rhs.bytecode;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_SHADER_HPP