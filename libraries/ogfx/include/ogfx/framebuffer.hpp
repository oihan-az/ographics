#ifndef OGFX_FRAMEBUFFER_HPP
#define OGFX_FRAMEBUFFER_HPP

#include <memory>
#include <vector>

namespace ogfx
{

class Device;
class RenderPass;
class ImageView;

struct FramebufferDesc
{
    const RenderPass* render_pass = nullptr;
    std::vector<const ImageView*> attachments;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers = 1;
};

class Framebuffer
{
  public:
    Framebuffer(const Device& device, const FramebufferDesc& desc);

    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&&) noexcept;
    Framebuffer& operator=(Framebuffer&&) noexcept;

  private:
    friend class CommandBuffer;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_FRAMEBUFFER_HPP