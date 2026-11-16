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

#include <omega/renderable/renderable.hpp>

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
class GpuMeshSystem;
class GpuTextureSystem;

template <typename T> class ComponentStorage;
template <typename Handle, typename Hash> class ResourceSetCache;

struct GeometryRenderFeatureDesc
{
    Entity camera;
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t frames_in_flight = 0;
    uint32_t* current_frame = nullptr;

    TextureHandle missing_texture;
    TextureViewHandle missing_texture_view;

    ComponentSystem& components;
    EventSystem& events;
    TransformSystem& transforms;
    CameraSystem& cameras;
    GpuMeshSystem& gpu_meshes;
    GpuTextureSystem& gpu_textures;
};

class GeometryRenderFeature : public orenderer::IRenderFeature
{
  public:
    GeometryRenderFeature(const GeometryRenderFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    const orendergraph::ResourceHandle<ogfx::Image> out_image() const { return m_out_image; }

  private:
    GeometryRenderFeatureDesc m_desc;

    FrameEventQueue<ComponentEvent<Renderable>> m_renderable_events;
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

        std::unordered_map<Entity, TextureHandle, EntityHash> m_renderable_textures;
        std::unique_ptr<ResourceSetCache<TextureHandle, TextureHandleHash>> m_texture_resource_sets;

        std::unordered_set<TextureHandle, TextureHandleHash> pending_textures;
        std::unordered_map<TextureHandle, TextureViewHandle, TextureHandleHash> m_texture_views;

        FrameResources(ComponentSystem& components, EventSystem& events)
            : directional_lights(components), point_lights(components), spot_lights(components)
        {}
    };

    std::vector<FrameResources> m_frame_resources;

    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;

    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;
};

} // namespace omega

#endif // GEOMETRY_RENDER_FEATURE_HPP