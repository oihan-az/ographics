#ifndef OGFX_RENDER_PASS_COMMANDS_HPP
#define OGFX_RENDER_PASS_COMMANDS_HPP

#include <cstdint>
#include <vector>

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

enum class ClearValueType
{
    Color,
    DepthStencil
};

struct ClearValue
{
    ClearValueType type = ClearValueType::Color;

    ClearColor color{};
    float depth = 1.0f;
    uint32_t stencil = 0;
};

struct RenderPassBeginInfo
{
    const RenderPass* render_pass = nullptr;
    const Framebuffer* framebuffer = nullptr;

    uint32_t offset_x = 0;
    uint32_t offset_y = 0;

    uint32_t width = 0;
    uint32_t height = 0;

    std::vector<ClearValue> clear_values = {ClearValue{}};
};

} // namespace ogfx

#endif // OGFX_RENDER_PASS_COMMANDS_HPP