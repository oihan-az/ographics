#ifndef ORENDERGRAPH_CONTEXT_BLIT_HPP
#define ORENDERGRAPH_CONTEXT_BLIT_HPP

#include <orendergraph/resource_handle.hpp>

#include <ogfx/image.hpp>

#include <cstdint>
#include <vector>

namespace orendergraph
{

class RenderGraph;

struct BlitContext
{
    struct Desc
    {
        ResourceHandle<ogfx::Image> source_image;
        std::vector<ResourceHandle<ogfx::Image>> destination_images;
        uint32_t destination_image_index = 0;

        uint32_t src_mip_level = 0;
        uint32_t src_base_array_layer = 0;

        uint32_t dst_mip_level = 0;
        uint32_t dst_base_array_layer = 0;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 1;
        uint32_t array_layer_count = 1;
    };

    void create(RenderGraph& graph, const Desc& desc);
    void clear();
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_TLAS_HPP
