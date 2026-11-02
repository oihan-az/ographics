#ifndef ORENDERGRAPH_OBJECT_TRAITS_GRAPHICS_PIPELINE_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_GRAPHICS_PIPELINE_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/graphics_pipeline.hpp>

#include <memory>

namespace orendergraph
{

template <> 
struct ObjectTraits<ogfx::GraphicsPipeline>
{
    using DescType = ogfx::GraphicsPipelineDesc;

    static std::unique_ptr<ogfx::GraphicsPipeline> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                          const DescType& desc)
    {
        return std::make_unique<ogfx::GraphicsPipeline>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        return lhs.render_pass == rhs.render_pass &&
               lhs.subpass == rhs.subpass &&
               lhs.layout == rhs.layout &&
               lhs.vertex_shader == rhs.vertex_shader &&
               lhs.fragment_shader == rhs.fragment_shader &&
               matches_vertex_input(lhs.vertex_input, rhs.vertex_input) &&
               matches_input_assembly(lhs.input_assembly, rhs.input_assembly) &&
               matches_rasterization(lhs.rasterization, rhs.rasterization) &&
               matches_multisampling(lhs.multisampling, rhs.multisampling) &&
               matches_color_blend(lhs.color_blend, rhs.color_blend) &&
               matches_viewport(lhs.viewport, rhs.viewport);
    }
    
private:  
    static bool matches_vertex_input(const ogfx::VertexInputStateDesc& lhs, const ogfx::VertexInputStateDesc& rhs)
    {
        if (lhs.bindings.size() != rhs.bindings.size() ||
            lhs.attributes.size() != rhs.attributes.size())
            return false;
    
        for (size_t i = 0; i < lhs.bindings.size(); ++i)
        {
            const auto& a = lhs.bindings[i];
            const auto& b = rhs.bindings[i];
    
            if (a.binding != b.binding ||
                a.stride != b.stride ||
                a.input_rate != b.input_rate)
                return false;
        }
    
        for (size_t i = 0; i < lhs.attributes.size(); ++i)
        {
            const auto& a = lhs.attributes[i];
            const auto& b = rhs.attributes[i];
    
            if (a.location != b.location ||
                a.binding != b.binding ||
                a.format != b.format ||
                a.offset != b.offset)
                return false;
        }
    
        return true;
    }
    
    static bool matches_input_assembly(const ogfx::InputAssemblyStateDesc& lhs, const ogfx::InputAssemblyStateDesc& rhs)
    {
        return lhs.topology == rhs.topology &&
               lhs.primitive_restart_enable == rhs.primitive_restart_enable;
    }
    
    static bool matches_rasterization(
        const ogfx::RasterizationStateDesc& lhs,
        const ogfx::RasterizationStateDesc& rhs)
    {
        return lhs.depth_clamp_enable == rhs.depth_clamp_enable &&
               lhs.rasterizer_discard_enable == rhs.rasterizer_discard_enable &&
               lhs.polygon_mode == rhs.polygon_mode &&
               lhs.cull_mode == rhs.cull_mode &&
               lhs.front_face == rhs.front_face &&
               lhs.depth_bias_enable == rhs.depth_bias_enable &&
               lhs.depth_bias_constant_factor == rhs.depth_bias_constant_factor &&
               lhs.depth_bias_clamp == rhs.depth_bias_clamp &&
               lhs.depth_bias_slope_factor == rhs.depth_bias_slope_factor &&
               lhs.line_width == rhs.line_width;
    }
    
    static bool matches_multisampling(const ogfx::MultisampleStateDesc& lhs, const ogfx::MultisampleStateDesc& rhs)
    {
        return lhs.rasterization_samples == rhs.rasterization_samples &&
               lhs.sample_shading_enable == rhs.sample_shading_enable &&
               lhs.min_sample_shading == rhs.min_sample_shading &&
               lhs.alpha_to_coverage_enable == rhs.alpha_to_coverage_enable &&
               lhs.alpha_to_one_enable == rhs.alpha_to_one_enable;
    }
    
    static bool matches_color_blend(const ogfx::ColorBlendStateDesc& lhs, const ogfx::ColorBlendStateDesc& rhs)
    {
        if (lhs.logic_op_enable != rhs.logic_op_enable ||
            lhs.attachments.size() != rhs.attachments.size())
            return false;
    
        for (size_t i = 0; i < lhs.attachments.size(); ++i)
        {
            const auto& a = lhs.attachments[i];
            const auto& b = rhs.attachments[i];
    
            if (a.blend_enable != b.blend_enable ||
                a.source_color_factor != b.source_color_factor ||
                a.destination_color_factor != b.destination_color_factor ||
                a.color_op != b.color_op ||
                a.source_alpha_factor != b.source_alpha_factor ||
                a.destination_alpha_factor != b.destination_alpha_factor ||
                a.alpha_op != b.alpha_op ||
                a.write_mask != b.write_mask)
                return false;
        }
    
        for (size_t i = 0; i < 4; ++i)
        {
            if (lhs.blend_constants[i] != rhs.blend_constants[i])
                return false;
        }
    
        return true;
    }
    
    static bool matches_viewport(const ogfx::ViewportStateDesc& lhs, const ogfx::ViewportStateDesc& rhs)
    {
        if (lhs.viewports.size() != rhs.viewports.size() ||
            lhs.scissors.size() != rhs.scissors.size() ||
            lhs.dynamic_viewport != rhs.dynamic_viewport ||
            lhs.dynamic_scissor != rhs.dynamic_scissor)
            return false;
    
        for (size_t i = 0; i < lhs.viewports.size(); ++i)
        {
            const auto& a = lhs.viewports[i];
            const auto& b = rhs.viewports[i];
    
            if (a.x != b.x ||
                a.y != b.y ||
                a.width != b.width ||
                a.height != b.height ||
                a.min_depth != b.min_depth ||
                a.max_depth != b.max_depth)
                return false;
        }
    
        for (size_t i = 0; i < lhs.scissors.size(); ++i)
        {
            const auto& a = lhs.scissors[i];
            const auto& b = rhs.scissors[i];
    
            if (a.x != b.x ||
                a.y != b.y ||
                a.width != b.width ||
                a.height != b.height)
                return false;
        }
    
        return true;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_GRAPHICS_PIPELINE_HPP