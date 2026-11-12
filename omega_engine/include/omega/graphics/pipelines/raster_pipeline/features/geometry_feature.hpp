#ifndef GEOMETRY_RENDER_FEATURE_HPP
#define GEOMETRY_RENDER_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>
#include <orendergraph/resource_descriptors.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/graphics/resources/dynamic_buffer.hpp>

#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/graphics/gpu_component/component_gpu_buffer.hpp>

#include <cstdint>

namespace ogfx
{
class Image;
class ImageView;
class CommandBuffer;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct RasterContext;
} // namespace orendergraph

namespace omega
{

class ComponentSystem;
class EventSystem;
class TransformSystem;
class CameraSystem;
class GpuMeshSystem;

template <typename T> class ComponentStorage;

struct GeometryRenderFeatureDesc
{
    Entity camera;
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t frames_in_flight = 0;
    uint32_t* current_frame = nullptr;

    ComponentSystem& components;
    EventSystem& events;
    TransformSystem& transforms;
    CameraSystem& cameras;
    GpuMeshSystem& gpu_meshes;
};

class GeometryRenderFeature : public orenderer::IRenderFeature
{
  public:
    GeometryRenderFeature(const GeometryRenderFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    const orendergraph::ResourceHandle<ogfx::Image> out_image() const { return m_out_image; }

  private:
    GeometryRenderFeatureDesc m_desc;

    ComponentGpuBuffer<DirectionalLight> m_directional_lights;
    ComponentGpuBuffer<PointLight> m_point_lights;
    ComponentGpuBuffer<SpotLight> m_spot_lights;

    void update_resource_set(DynamicBuffer& buffer, orendergraph::ResourceWriteDesc& write_desc,
                             orendergraph::RenderGraph& rg, orendergraph::RenderGraphNode<orendergraph::RasterContext>& node);

    struct FrameResource
    {
        orendergraph::ResourceWriteDesc directional_buffer_write;
        orendergraph::ResourceWriteDesc point_buffer_write;
        orendergraph::ResourceWriteDesc spot_buffer_write;
    };
    
    std::vector<FrameResource> m_frames;

    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;
};

} // namespace omege

#endif // GEOMETRY_RENDER_FEATURE_HPP