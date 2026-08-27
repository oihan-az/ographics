#ifndef OGFX_RENDER_PASS_HPP
#define OGFX_RENDER_PASS_HPP

#include <ogfx/format.hpp>
#include <ogfx/image_state.hpp>
#include <ogfx/load_store_op.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class Device;

struct RenderPassAttachmentDesc
{
    Format format = Format::Undefined;

    LoadOp load_op = LoadOp::Clear;
    StoreOp store_op = StoreOp::Store;

    ImageState initial_state = ImageState::Undefined;
    ImageState final_state = ImageState::Present;
};

struct RenderPassDesc
{
    std::vector<RenderPassAttachmentDesc> attachments;
};

class RenderPass
{
  public:
    RenderPass(const Device& device, const RenderPassDesc& desc);
    ~RenderPass();

    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    RenderPass(RenderPass&&) noexcept;
    RenderPass& operator=(RenderPass&&) noexcept;

  private:
    friend class Framebuffer;
    friend class CommandBuffer;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_RENDER_PASS_HPP