#ifndef OGFX_COMMAND_BUFFER_HPP
#define OGFX_COMMAND_BUFFER_HPP

#include <ogfx/image_state.hpp>
#include <ogfx/shader_stage.hpp>
#include <ogfx/pipeline_type.hpp>

#include <memory>

namespace ogfx
{

class CommandPool;
class RenderPass;
class Framebuffer;
class GraphicsPipeline;
class ComputePipeline;
class Buffer;
class ResourceSet;
class PipelineLayout;
class Image;

struct ClearColor
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct RenderPassBeginInfo
{
    uint32_t offset_x = 0;
    uint32_t offset_y = 0;

    uint32_t width = 0;
    uint32_t height = 0;

    ClearColor clear_color{};
};

struct DrawInfo
{
    uint32_t vertex_count = 0;
    uint32_t instance_count = 1;
    uint32_t first_vertex = 0;
    uint32_t first_instance = 0;
};

struct DispatchInfo
{
    uint32_t group_count_x = 1;
    uint32_t group_count_y = 1;
    uint32_t group_count_z = 1;
};

struct VertexBufferBindInfo
{
    const Buffer* buffer = nullptr;
    uint32_t binding = 0;
    uint64_t offset = 0;
};

struct ResourceSetBindInfo
{
    const PipelineLayout* layout = nullptr;
    const ResourceSet* resource_set = nullptr;
    uint32_t set_index = 0;
    PipelineType pipeline_type = PipelineType::Graphics;
};

struct InlineConstantsInfo
{
    const PipelineLayout* layout = nullptr;
    ShaderStageFlags stages = ShaderStageFlags::None;
    uint32_t offset = 0;
    uint32_t size = 0;
    const void* data = nullptr;
};

struct ImageTransitionInfo
{
    const Image* image = nullptr;

    ImageState old_state = ImageState::Undefined;
    ImageState new_state = ImageState::Undefined;

    uint32_t base_mip_level = 0;
    uint32_t mip_level_count = 1;

    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;
};

struct BufferToImageCopyInfo
{
    const Buffer* buffer = nullptr;
    const Image* image = nullptr;

    uint64_t buffer_offset = 0;
    uint32_t buffer_row_length = 0;
    uint32_t buffer_image_height = 0;

    uint32_t mip_level = 0;
    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
};

struct GenerateMipmapsInfo
{
    const Image* image = nullptr;
};

class CommandBuffer
{
  public:
    explicit CommandBuffer(const CommandPool& command_pool);

    ~CommandBuffer();

    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    CommandBuffer(CommandBuffer&&) noexcept;
    CommandBuffer& operator=(CommandBuffer&&) noexcept;

    void begin();
    void end();
    void reset();

    void begin_render_pass(const RenderPass& render_pass, const Framebuffer& framebuffer,
                           const RenderPassBeginInfo& begin_info);
    void end_render_pass();

    void bind_graphics_pipeline(const GraphicsPipeline& pipeline);

    void draw(const DrawInfo& draw_info);

    void bind_compute_pipeline(const ComputePipeline& pipeline);

    void dispatch(const DispatchInfo& dispatch_info);

    void bind_vertex_buffer(const VertexBufferBindInfo& info);

    void bind_resource_set(const ResourceSetBindInfo& info);
    void set_inline_constants(const InlineConstantsInfo& info);

    void transition_image(const ImageTransitionInfo& info);
    void copy_buffer_to_image(const BufferToImageCopyInfo& info);

    void generate_mipmaps(const GenerateMipmapsInfo& info);

  private:
    friend class Queue;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_COMMAND_BUFFER_HPP