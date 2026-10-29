#ifndef OGFX_COMMAND_BUFFER_HPP
#define OGFX_COMMAND_BUFFER_HPP

#include <ogfx/commands/render_pass_commands.hpp>
#include <ogfx/commands/binding_commands.hpp>
#include <ogfx/commands/draw_commands.hpp>
#include <ogfx/commands/dispatch_commands.hpp>
#include <ogfx/commands/inline_constants_commands.hpp>
#include <ogfx/commands/transfer_commands.hpp>
#include <ogfx/commands/barrier_commands.hpp>
#include <ogfx/commands/acc_struct_commands.hpp>
#include <ogfx/commands/ray_tracing_commands.hpp>

#include <memory>

namespace ogfx
{

class CommandPool;

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

    void begin_render_pass(const RenderPassBeginInfo& begin_info);
    void end_render_pass();

    void bind_pipeline(const PipelineBindInfo& bind_info);
    void bind_vertex_buffer(const VertexBufferBindInfo& bind_info);
    void bind_index_buffer(const IndexBufferBindInfo& bind_info);
    void bind_resource_set(const ResourceSetBindInfo& info);

    void draw(const DrawInfo& draw_info);
    void draw_indexed(const DrawIndexedInfo& draw_info);
    void draw_indexed_indirect(const DrawIndexedIndirectInfo& draw_info);

    void dispatch(const DispatchInfo& dispatch_info);

    void set_inline_constants(const InlineConstantsInfo& info);

    void copy_buffer_to_image(const BufferToImageCopyInfo& info);
    void copy_image_to_image(const ImageToImageCopyInfo& info);
    void blit_image(const ImageToImageCopyInfo& info);

    void generate_mipmaps(const GenerateMipmapsInfo& info);

    void memory_barrier(const MemoryBarrierInfo& info);
    void buffer_barrier(const BufferBarrierInfo& info);
    void image_barrier(const ImageBarrierInfo& info);
    void transition_image(const ImageTransitionInfo& info);

    void build_acceleration_structure(const AccelerationStructureBuildInfo& info);

    void trace_rays(const TraceRaysInfo& info);

  private:
    friend class Queue;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_COMMAND_BUFFER_HPP