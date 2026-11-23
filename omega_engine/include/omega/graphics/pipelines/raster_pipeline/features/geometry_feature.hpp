#ifndef GEOMETRY_RENDER_FEATURE_HPP
#define GEOMETRY_RENDER_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>
#include <orendergraph/resource_descriptors.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/core/component/component_event.hpp>

#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/renderable/pbr_opaque_renderable.hpp>
#include <omega/material/materials/pbr_opaque_material.hpp>
#include <omega/material/material_handle.hpp>

#include <omega/camera/camera_type.hpp>

#include <omega/graphics/resources/gpu_dynamic_buffer.hpp>
#include <omega/graphics/resources/gpu_component_buffer.hpp>
#include <omega/graphics/resources/resource_set_cache.hpp>
#include <omega/texture/texture_handle.hpp>
#include <omega/texture/texture_view_handle.hpp>

#include <omega/frames_in_flight/frame_event_queue.hpp>

#include <cstdint>
#include <unordered_set>

namespace ogfx
{
class Image;
class ImageView;
class CommandBuffer;
class Sampler;
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
class GraphicsSystem;

template <typename T> class ComponentStorage;
template <typename Handle, typename Hash> class ResourceSetCache;

struct PbrOpaqueRenderFeatureDesc
{
    Entity camera;
    CameraType camera_type;

    ComponentSystem& components;
    EventSystem& events;
    TransformSystem& transforms;
    CameraSystem& cameras;
    GraphicsSystem& graphics;
};

class PbrOpaqueRenderFeature : public orenderer::IRenderFeature
{
  public:
    PbrOpaqueRenderFeature(const PbrOpaqueRenderFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void on_swapchain_recreated(orenderer::RenderFeatureContext& ctx) override;

    void set_camera(omega::Entity camera, CameraType camera_type);

    const orendergraph::ResourceHandle<ogfx::Image> out_image() const { return m_out_image; }

  private:
    using PbrMaterialHandle = MaterialHandle<PbrOpaqueMaterial>;
    using PbrMaterialHash = MaterialHandleHash<PbrOpaqueMaterial>;
    PbrMaterialHandle m_default_material{};

    PbrOpaqueRenderFeatureDesc m_desc;

    FrameEventQueue<ComponentEvent<PbrOpaqueRenderable>> m_renderable_events;
    FrameEventQueue<ComponentEvent<DirectionalLight>> m_directional_light_events;
    FrameEventQueue<ComponentEvent<PointLight>> m_point_light_events;
    FrameEventQueue<ComponentEvent<SpotLight>> m_spot_light_events;

    struct FrameResources
    {
        ComponentGpuBuffer<DirectionalLight> directional_lights;
        ComponentGpuBuffer<PointLight> point_lights;
        ComponentGpuBuffer<SpotLight> spot_lights;

        orendergraph::ResourceWriteDesc directional_buffer_write;
        orendergraph::ResourceWriteDesc point_buffer_write;
        orendergraph::ResourceWriteDesc spot_buffer_write;

        std::unordered_map<Entity, PbrMaterialHandle, EntityHash> renderable_materials;
        std::unique_ptr<ResourceSetCache<PbrMaterialHandle, PbrMaterialHash>> material_resource_sets;

        std::unordered_set<PbrMaterialHandle, PbrMaterialHash> pending_materials;

        FrameResources(ComponentSystem& components, EventSystem& events)
            : directional_lights(components), point_lights(components), spot_lights(components)
        {}
    };

    std::vector<FrameResources> m_frame_resources;
    std::unordered_map<TextureHandle, TextureViewHandle, TextureHandleHash> m_texture_views;

    void create_output_resources(orendergraph::RenderGraph& rg);
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* m_geometry_node = nullptr;

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;

    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;
};

} // namespace omega

#endif // GEOMETRY_RENDER_FEATURE_HPP