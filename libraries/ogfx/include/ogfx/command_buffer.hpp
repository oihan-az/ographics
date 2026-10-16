#ifndef OGFX_COMMAND_BUFFER_HPP
#define OGFX_COMMAND_BUFFER_HPP

#include <memory>

namespace ogfx
{

class CommandPool;
class RenderPass;
class Framebuffer;
class GraphicsPipeline;
class Buffer;

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

struct VertexBufferBindInfo
{
    const Buffer* buffer = nullptr;
    uint32_t binding = 0;
    uint64_t offset = 0;
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

    void bind_vertex_buffer(const VertexBufferBindInfo& info);

  private:
    friend class Queue;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_COMMAND_BUFFER_HPP