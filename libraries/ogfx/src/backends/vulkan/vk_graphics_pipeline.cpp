#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_graphics_pipeline.hpp"
#include "vk_device.hpp"
#include "vk_render_pass.hpp"
#include "vk_pipeline_layout.hpp"
#include "vk_shader.hpp"
#include "vk_shader_stage.hpp"
#include "vk_format.hpp"
#include "vk_input_assembly.hpp"
#include "vk_rasterization.hpp"
#include "vk_multisample.hpp"
#include "vk_color_blend.hpp"
#include "vk_viewport.hpp"

#include <stdexcept>

namespace ogfx
{

GraphicsPipeline::GraphicsPipeline(const Device& device, const GraphicsPipelineDesc& desc)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    if (desc.render_pass == nullptr)
    {
        throw std::invalid_argument("Graphics pipeline requires a render pass.");
    }

    if (desc.shaders.empty())
    {
        throw std::invalid_argument("Graphics pipeline requires a layout.");
    }

    if (desc.shaders.empty())
    {
        throw std::invalid_argument("Graphics pipeline requires shader stages.");
    }

    OGFX_LOG("Creating graphics pipeline with " + std::to_string(desc.shaders.size()) + " shader stage(s)");


    // Collect the Vulkan shader stages.
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    shader_stages.reserve(desc.shaders.size());

    bool has_vertex_shader = false;
    bool has_fragment_shader = false;

    for (const Shader* shader : desc.shaders)
    {
        if (shader == nullptr || !shader->m_impl)
        {
            throw std::invalid_argument("Graphics pipeline contains an invalid shader.");
        }

        if (shader->m_stage == ShaderStage::Vertex)
        {
            if (has_vertex_shader)
            {
                throw std::invalid_argument("Graphics pipeline contains multiple vertex shaders.");
            }

            has_vertex_shader = true;
        }
        else if (shader->m_stage == ShaderStage::Fragment)
        {
            if (has_fragment_shader)
            {
                throw std::invalid_argument("Graphics pipeline contains multiple fragment shaders.");
            }

            has_fragment_shader = true;
        }
        else if (shader->m_stage == ShaderStage::Compute)
        {
            throw std::invalid_argument("Compute shaders cannot be used in a graphics pipeline.");
        }

        VkPipelineShaderStageCreateInfo stage_info{};
        stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage_info.stage = to_vk_shader_stage(shader->m_stage);
        stage_info.module = shader->m_impl->m_shader_module;
        stage_info.pName = shader->m_entry_point.c_str();

        shader_stages.push_back(stage_info);

        OGFX_LOG("Added " + to_string_enum(shader->m_stage) + " shader stage, entry point: '" +
                 shader->m_entry_point + "'");
    }

    if (!has_vertex_shader)
    {
        throw std::invalid_argument("Graphics pipeline requires a vertex shader.");
    }

    if (!has_fragment_shader)
    {
        throw std::invalid_argument("Graphics pipeline requires a fragment shader.");
    }

    // Vertex input
    OGFX_LOG("Creating vertex input state:");
    OGFX_LOG("  Binding count: " + std::to_string(desc.vertex_input.bindings.size()));
    OGFX_LOG("  Attribute count: " + std::to_string(desc.vertex_input.attributes.size()));

    std::vector<VkVertexInputBindingDescription> vertex_bindings;
    vertex_bindings.reserve(desc.vertex_input.bindings.size());

    for (size_t i = 0; i < desc.vertex_input.bindings.size(); ++i)
    {
        const VertexBindingDesc& binding = desc.vertex_input.bindings[i];

        OGFX_LOG("  Vertex binding " + std::to_string(i) + ":");
        OGFX_LOG("    Binding: " + std::to_string(binding.binding));
        OGFX_LOG("    Stride: " + std::to_string(binding.stride));
        OGFX_LOG("    Input rate: " + to_string_enum(binding.input_rate));

        VkVertexInputBindingDescription vk_binding{};
        vk_binding.binding = binding.binding;
        vk_binding.stride = binding.stride;

        switch (binding.input_rate)
        {
        case VertexInputRate::Vertex:
            vk_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            break;

        case VertexInputRate::Instance:
            vk_binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            break;
        }

        vertex_bindings.push_back(vk_binding);
    }

    std::vector<VkVertexInputAttributeDescription> vertex_attributes;
    vertex_attributes.reserve(desc.vertex_input.attributes.size());

    for (size_t i = 0; i < desc.vertex_input.attributes.size(); ++i)
    {
        const VertexAttributeDesc& attribute = desc.vertex_input.attributes[i];

        OGFX_LOG("  Vertex attribute " + std::to_string(i) + ":");
        OGFX_LOG("    Location: " + std::to_string(attribute.location));
        OGFX_LOG("    Binding: " + std::to_string(attribute.binding));
        OGFX_LOG("    Format: " + to_string_enum(attribute.format));
        OGFX_LOG("    Offset: " + std::to_string(attribute.offset));

        VkVertexInputAttributeDescription vk_attribute{};
        vk_attribute.location = attribute.location;
        vk_attribute.binding = attribute.binding;
        vk_attribute.format = to_vk_format(attribute.format);
        vk_attribute.offset = attribute.offset;

        vertex_attributes.push_back(vk_attribute);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_bindings.size());
    vertex_input.pVertexBindingDescriptions = vertex_bindings.empty() ? nullptr : vertex_bindings.data();
    vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes.size());
    vertex_input.pVertexAttributeDescriptions = vertex_attributes.empty() ? nullptr : vertex_attributes.data();

    // Input assembly.
    OGFX_LOG("Creating input assembly state:");
    OGFX_LOG("  Topology: " + to_string_enum(desc.input_assembly.topology));
    OGFX_LOG("  Primitive restart: " + std::string(desc.input_assembly.primitive_restart_enable ? "true" : "false"));

    if (desc.input_assembly.topology == PrimitiveTopology::Undefined)
    {
        OGFX_LOG_ERROR("Graphics pipeline contains an invalid primitive topology");
        throw std::invalid_argument("Graphics pipeline contains an invalid primitive topology.");
    }

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = to_vk_primitive_topology(desc.input_assembly.topology);
    input_assembly.primitiveRestartEnable = desc.input_assembly.primitive_restart_enable ? VK_TRUE : VK_FALSE;

    // Rasterization
    OGFX_LOG("Creating rasterization state:");
    OGFX_LOG("  Polygon mode: " + to_string_enum(desc.rasterization.polygon_mode));
    OGFX_LOG("  Cull mode: " + to_string_enum(desc.rasterization.cull_mode));
    OGFX_LOG("  Front face: " + to_string_enum(desc.rasterization.front_face));

    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    rasterization.depthClampEnable = desc.rasterization.depth_clamp_enable ? VK_TRUE : VK_FALSE;
    rasterization.rasterizerDiscardEnable = desc.rasterization.rasterizer_discard_enable ? VK_TRUE : VK_FALSE;

    rasterization.polygonMode = to_vk_polygon_mode(desc.rasterization.polygon_mode);
    rasterization.cullMode = to_vk_cull_mode(desc.rasterization.cull_mode);
    rasterization.frontFace = to_vk_front_face(desc.rasterization.front_face);

    rasterization.depthBiasEnable = desc.rasterization.depth_bias_enable ? VK_TRUE : VK_FALSE;
    rasterization.depthBiasConstantFactor = desc.rasterization.depth_bias_constant_factor;
    rasterization.depthBiasClamp = desc.rasterization.depth_bias_clamp;
    rasterization.depthBiasSlopeFactor = desc.rasterization.depth_bias_slope_factor;

    rasterization.lineWidth = desc.rasterization.line_width;

    // Multisample
    OGFX_LOG("Creating multisampling state:");
    OGFX_LOG("  Rasterization samples: " + to_string_enum(desc.multisampling.rasterization_samples));
    OGFX_LOG("  Sample shading: " + std::string(desc.multisampling.sample_shading_enable ? "true" : "false"));
    OGFX_LOG("  Min sample shading: " + std::to_string(desc.multisampling.min_sample_shading));
    OGFX_LOG("  Alpha to coverage: " + std::string(desc.multisampling.alpha_to_coverage_enable ? "true" : "false"));
    OGFX_LOG("  Alpha to one: " + std::string(desc.multisampling.alpha_to_one_enable ? "true" : "false"));

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    multisampling.rasterizationSamples = to_vk_sample_count(desc.multisampling.rasterization_samples);
    multisampling.sampleShadingEnable = desc.multisampling.sample_shading_enable ? VK_TRUE : VK_FALSE;
    multisampling.minSampleShading = desc.multisampling.min_sample_shading;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = desc.multisampling.alpha_to_coverage_enable ? VK_TRUE : VK_FALSE;
    multisampling.alphaToOneEnable = desc.multisampling.alpha_to_one_enable ? VK_TRUE : VK_FALSE;

    // Color blend
    OGFX_LOG("Creating color blend state:");
    OGFX_LOG("  Attachment count: " + std::to_string(desc.color_blend.attachments.size()));

    std::vector<VkPipelineColorBlendAttachmentState> blend_attachments;
    blend_attachments.reserve(desc.color_blend.attachments.size());

    for (size_t i = 0; i < desc.color_blend.attachments.size(); ++i)
    {
        const ColorBlendAttachmentDesc& attachment = desc.color_blend.attachments[i];

        OGFX_LOG("  Color blend attachment " + std::to_string(i) + ":");
        OGFX_LOG("    Blend enabled: " + std::string(attachment.blend_enable ? "true" : "false"));
        OGFX_LOG("    Color op: " + to_string_enum(attachment.color_op));
        OGFX_LOG("    Alpha op: " + to_string_enum(attachment.alpha_op));
        OGFX_LOG("    Write mask: " + to_string_bitmask(attachment.write_mask));

        VkPipelineColorBlendAttachmentState vk_attachment{};
        vk_attachment.blendEnable = attachment.blend_enable ? VK_TRUE : VK_FALSE;
        vk_attachment.srcColorBlendFactor = to_vk_blend_factor(attachment.source_color_factor);
        vk_attachment.dstColorBlendFactor = to_vk_blend_factor(attachment.destination_color_factor);
        vk_attachment.colorBlendOp = to_vk_blend_op(attachment.color_op);
        vk_attachment.srcAlphaBlendFactor = to_vk_blend_factor(attachment.source_alpha_factor);
        vk_attachment.dstAlphaBlendFactor = to_vk_blend_factor(attachment.destination_alpha_factor);
        vk_attachment.alphaBlendOp = to_vk_blend_op(attachment.alpha_op);
        vk_attachment.colorWriteMask = to_vk_color_components(attachment.write_mask);

        blend_attachments.push_back(vk_attachment);
    }

    VkPipelineColorBlendStateCreateInfo color_blend{};
    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.logicOpEnable = desc.color_blend.logic_op_enable ? VK_TRUE : VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_COPY;
    color_blend.attachmentCount = static_cast<uint32_t>(blend_attachments.size());
    color_blend.pAttachments = blend_attachments.empty() ? nullptr : blend_attachments.data();

    for (uint32_t i = 0; i < 4; ++i)
    {
        color_blend.blendConstants[i] = desc.color_blend.blend_constants[i];
    }

    // Viewport and scissor state
    OGFX_LOG("Creating viewport state:");
    OGFX_LOG("  Dynamic viewport: " + std::string(desc.viewport.dynamic_viewport ? "true" : "false"));
    OGFX_LOG("  Dynamic scissor: " + std::string(desc.viewport.dynamic_scissor ? "true" : "false"));

    std::vector<VkViewport> viewports;
    if (!desc.viewport.dynamic_viewport)
    {
        viewports.reserve(desc.viewport.viewports.size());

        for (const Viewport& viewport : desc.viewport.viewports)
            viewports.push_back(to_vk_viewport(viewport));
    }

    std::vector<VkRect2D> scissors;
    if (!desc.viewport.dynamic_scissor)
    {
        scissors.reserve(desc.viewport.scissors.size());

        for (const Scissor& scissor : desc.viewport.scissors)
            scissors.push_back(to_vk_scissor(scissor));
    }

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = desc.viewport.dynamic_viewport ? 1 : static_cast<uint32_t>(viewports.size());
    viewport_state.pViewports = (!desc.viewport.dynamic_viewport && !viewports.empty()) ? viewports.data() : nullptr;
    viewport_state.scissorCount = desc.viewport.dynamic_scissor ? 1 : static_cast<uint32_t>(scissors.size());
    viewport_state.pScissors = (!desc.viewport.dynamic_scissor && !scissors.empty()) ? scissors.data() : nullptr;

    std::vector<VkDynamicState> dynamic_states;

    if (desc.viewport.dynamic_viewport)
        dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);

    if (desc.viewport.dynamic_scissor)
        dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.empty() ? nullptr : dynamic_states.data();

    OGFX_LOG("Viewport state created:");
    OGFX_LOG("  Viewport count: " + std::to_string(viewport_state.viewportCount));
    OGFX_LOG("  Scissor count: " + std::to_string(viewport_state.scissorCount));

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterization;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blend;
    pipeline_info.pDynamicState = dynamic_states.empty() ? nullptr : &dynamic_state;
    pipeline_info.layout = desc.layout->m_impl->m_pipeline_layout;
    pipeline_info.renderPass = desc.render_pass->m_impl->m_render_pass;
    pipeline_info.subpass = desc.subpass;

    if (vkCreateGraphicsPipelines(m_impl->m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_impl->m_pipeline) !=
        VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create graphics pipeline");
        throw std::runtime_error("Failed to create graphics pipeline.");
    }

    OGFX_LOG("Graphics pipeline created");
}

GraphicsPipeline::~GraphicsPipeline()
{
    if (!m_impl)
    {
        return;
    }

    if (m_impl->m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(m_impl->m_device, m_impl->m_pipeline, nullptr);

        m_impl->m_pipeline = VK_NULL_HANDLE;

        OGFX_LOG("Graphics pipeline destroyed");
    }
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&&) noexcept = default;
GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&&) noexcept = default;

} // namespace ogfx