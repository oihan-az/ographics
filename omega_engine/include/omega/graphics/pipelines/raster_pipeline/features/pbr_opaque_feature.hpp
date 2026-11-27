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

#include <omega/shadows/directional_shadow.hpp>
#include <omega/shadows/point_shadow.hpp>
#include <omega/shadows/spot_shadow.hpp>

#include <omega/renderable/pbr_opaque_renderable.hpp>
#include <omega/material/materials/pbr_opaque_material.hpp>
#include <omega/material/material_handle.hpp>

#include <omega/camera/camera_type.hpp>

#include <omega/transform/transform.hpp>

#include <omega/graphics/resources/gpu_dynamic_buffer.hpp>
#include <omega/graphics/resources/gpu_component_buffer.hpp>
#include <omega/graphics/resources/resource_set_cache.hpp>
#include <omega/texture/texture_handle.hpp>
#include <omega/texture/texture_view_handle.hpp>

#include <omega/frames_in_flight/frame_event_queue.hpp>

#include <cstdint>
#include <unordered_set>
#include <optional>

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
class ShadowMapGen;

template <typename T> class ComponentStorage;
template <typename Handle, typename Hash> class ResourceSetCache;

struct ShadowTextureTable
{
    struct Entry
    {
        uint32_t index = 0;
        orendergraph::ResourceHandle<ogfx::ImageView> view;
    };

    std::unordered_map<Entity, Entry, EntityHash> entries;

    std::vector<uint32_t> free_indices;
    uint32_t next_index = 0;

    std::unordered_set<Entity, EntityHash> pending;

    std::optional<uint32_t> allocate_slot(uint32_t maximum)
    {
        if (!free_indices.empty())
        {
            const uint32_t index = free_indices.back();
            free_indices.pop_back();
            return index;
        }

        if (next_index >= maximum)
            return std::nullopt;

        return next_index++;
    }

    std::optional<uint32_t> find(Entity entity) const
    {
        const auto it = entries.find(entity);

        if (it == entries.end())
            return std::nullopt;

        return it->second.index;
    }
};

struct ShadowResourceArray
{
    uint32_t capacity = 1;
    uint32_t resource_set_index = std::numeric_limits<uint32_t>::max();
};

struct PbrOpaqueRenderFeatureDesc
{
    Entity camera;
    CameraType camera_type;

    orendergraph::ResourceHandle<ogfx::Image> irradiance_map;
    orendergraph::ResourceHandle<ogfx::ImageView> irradiance_map_view;

    orendergraph::ResourceHandle<ogfx::Image> environment_map;
    orendergraph::ResourceHandle<ogfx::ImageView> environment_map_view;

    orendergraph::ResourceHandle<ogfx::Image> brdf_lut;
    orendergraph::ResourceHandle<ogfx::ImageView> brdf_lut_view;

    ShadowMapGen& shadows;

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

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* node() const
    {
        return m_pbr_node;
    }

    const orendergraph::ResourceHandle<ogfx::Image> out_image() const { return m_out_image; }
    const orendergraph::ResourceHandle<ogfx::ImageView> out_image_view() const { return m_out_image_view; }
    orendergraph::ResourceHandle<ogfx::Image> depth_image() const { return m_depth_image; }
    orendergraph::ResourceHandle<ogfx::ImageView> depth_image_view() const { return m_depth_image_view; }

  private:
    using PbrMaterialHandle = MaterialHandle<PbrOpaqueMaterial>;
    using PbrMaterialHash = MaterialHandleHash<PbrOpaqueMaterial>;
    PbrMaterialHandle m_default_material{};

    PbrOpaqueRenderFeatureDesc m_desc;

    FrameEventQueue<ComponentEvent<PbrOpaqueRenderable>> m_renderable_events;
    FrameEventQueue<ComponentEvent<DirectionalLight>> m_directional_light_events;
    FrameEventQueue<ComponentEvent<PointLight>> m_point_light_events;
    FrameEventQueue<ComponentEvent<SpotLight>> m_spot_light_events;
    FrameEventQueue<ComponentEvent<DirectionalShadow>> m_directional_shadow_events;
    FrameEventQueue<ComponentEvent<PointShadow>> m_point_shadow_events;
    FrameEventQueue<ComponentEvent<SpotShadow>> m_spot_shadow_events;
    FrameEventQueue<ComponentEvent<Transform>> m_transform_events;

    struct FrameResources
    {
        ComponentGpuBuffer<DirectionalLight> directional_lights;
        ComponentGpuBuffer<PointLight> point_lights;
        ComponentGpuBuffer<SpotLight> spot_lights;

        ComponentGpuBuffer<DirectionalShadow> directional_shadows;
        ComponentGpuBuffer<PointShadow> point_shadows;
        ComponentGpuBuffer<SpotShadow> spot_shadows;

        DynamicBuffer directional_shadow_lookup;
        DynamicBuffer point_shadow_lookup;
        DynamicBuffer spot_shadow_lookup;

        orendergraph::ResourceWriteDesc directional_buffer_write;
        orendergraph::ResourceWriteDesc point_buffer_write;
        orendergraph::ResourceWriteDesc spot_buffer_write;

        orendergraph::ResourceWriteDesc directional_shadow_buffer_write;
        orendergraph::ResourceWriteDesc point_shadow_buffer_write;
        orendergraph::ResourceWriteDesc spot_shadow_buffer_write;

        orendergraph::ResourceWriteDesc directional_shadow_lookup_write;
        orendergraph::ResourceWriteDesc point_shadow_lookup_write;
        orendergraph::ResourceWriteDesc spot_shadow_lookup_write;

        uint32_t shadow_data_resource_set_index = std::numeric_limits<uint32_t>::max();

        ShadowResourceArray directional_shadow_resources;
        ShadowResourceArray spot_shadow_resources;
        ShadowResourceArray point_shadow_resources;

        ShadowTextureTable directional_shadow_textures;
        ShadowTextureTable point_shadow_textures;
        ShadowTextureTable spot_shadow_textures;

        std::unordered_map<Entity, PbrMaterialHandle, EntityHash> renderable_materials;
        std::unique_ptr<ResourceSetCache<PbrMaterialHandle, PbrMaterialHash>> material_resource_sets;

        std::unordered_set<PbrMaterialHandle, PbrMaterialHash> pending_materials;

        FrameResources(ComponentSystem& components, EventSystem& events)
            : directional_lights(components), point_lights(components), spot_lights(components),
              directional_shadows(components), point_shadows(components), spot_shadows(components)
        {}
    };

    uint32_t m_max_directional_shadow_descriptors = 0;
    uint32_t m_max_spot_shadow_descriptors = 0;
    uint32_t m_max_point_shadow_descriptors = 0;

    std::vector<FrameResources> m_frame_resources;
    std::unordered_map<TextureHandle, TextureViewHandle, TextureHandleHash> m_texture_views;

    void create_output_resources(orendergraph::RenderGraph& rg);
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* m_pbr_node = nullptr;

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;

    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;
    orendergraph::ResourceHandle<ogfx::Sampler> m_environment_sampler;
    orendergraph::ResourceHandle<ogfx::Sampler> m_brdf_lut_sampler;
    orendergraph::ResourceHandle<ogfx::Sampler> m_shadow_sampler;
};

} // namespace omega

#endif // GEOMETRY_RENDER_FEATURE_HPP