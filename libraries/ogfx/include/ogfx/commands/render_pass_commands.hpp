#ifndef OGFX_RENDER_PASS_COMMANDS_HPP
#define OGFX_RENDER_PASS_COMMANDS_HPP

#include <cstdint>

namespace ogfx
{

class RenderPass;
class Framebuffer;

struct ClearColor
{
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct RenderPassBeginInfo
{
    const RenderPass* render_pass;
    const Framebuffer* framebuffer;

    uint32_t offset_x = 0;
    uint32_t offset_y = 0;

    uint32_t width = 0;
    uint32_t height = 0;

    ClearColor clear_color{};
};

} // namespace ogfx

#endif // OGFX_RENDER_PASS_COMMANDS_HPP