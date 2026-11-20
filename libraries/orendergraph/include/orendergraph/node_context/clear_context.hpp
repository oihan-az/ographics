#ifndef ORENDERGRAPH_CLEAR_CONTEXT_HPP
#define ORENDERGRAPH_CLEAR_CONTEXT_HPP

#include <orendergraph/resource_handle.hpp>

#include <ogfx/image.hpp>

#include <cstdint>
#include <vector>

namespace orendergraph
{

struct ClearContext
{
    struct Desc
    {
        std::vector<ResourceHandle<ogfx::Image>> images;

        uint32_t image_index = 0;

        float color[4] = {0.08f, 0.08f, 0.09f, 1.0f};
    };
};

} // namespace orendergraph

#endif