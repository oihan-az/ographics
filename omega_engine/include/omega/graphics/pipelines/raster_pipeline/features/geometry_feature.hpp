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

#include <omega/renderable/renderable.hpp>

#include <omega/graphics/gpu_component/component_gpu_buffer.hpp>
#include <omega/graphics/resources/resource_set_cache.hpp>
#include <omega/texture/texture_handle.hpp>
#include <omega/texture/texture_view_handle.hpp>

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

    struct FrameResourceSets
    {
        std::unique_ptr<ResourceSetCache<TextureHandle, TextureHandleHash>> m_texture_resource_sets;
        std::unordered_set<TextureHandle, TextureHandleHash> pending_textures;

        std::vector<ComponentAddedEvent<Renderable>> pending_added;
        std::vector<ComponentRemovedEvent<Renderable>> pending_removed;
        std::vector<ComponentModifiedEvent<Renderable>> pending_modified;
    };

    std::vector<FrameResourceSets> m_resource_sets;
    std::unordered_map<TextureHandle, TextureViewHandle, TextureHandleHash> m_texture_views;

    void update_texture_resource_set(FrameResourceSets& frame, TextureHandle texture);

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

    orendergraph::ResourceHandle<ogfx::Sampler> m_sampler;
};

} // namespace omege

#endif // GEOMETRY_RENDER_FEATURE_HPP