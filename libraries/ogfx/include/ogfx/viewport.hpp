#ifndef OGFX_VIEWPORT_HPP
#define OGFX_VIEWPORT_HPP

#include <cstdint>
#include <vector>

namespace ogfx
{

struct Viewport
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float min_depth = 0.0f;
    float max_depth = 1.0f;
};

struct Scissor
{
    int32_t x = 0;
    int32_t y = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct ViewportStateDesc
{
    std::vector<Viewport> viewports;
    std::vector<Scissor> scissors;

    bool dynamic_viewport = true;
    bool dynamic_scissor = true;
};

} // namespace ogfx

#endif // OGFX_VIEWPORT_HPP