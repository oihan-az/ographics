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

    template <typename T>
    struct LightFrameResources
    {
        DynamicBuffer buffer;
    
        orendergraph::ResourceWriteDesc buffer_write;

        std::vector<ComponentAddedEvent<T>> pending_added;
        std::vector<ComponentRemovedEvent<T>> pending_removed;
        std::vector<ComponentModifiedEvent<T>> pending_modified;

        bool descriptor_update_pending = false;
    };
    
    struct FrameResources
    {
        LightFrameResources<DirectionalLight> directional;
        LightFrameResources<PointLight> point;
        LightFrameResources<SpotLight> spot;

        template <typename T> LightFrameResources<T>& get();
    };

    std::vector<FrameResources> m_frames;

    template <typename T> void capture_light_events();
    template <typename T> void process_light_events(uint32_t frame_index, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg);
    template <typename T> void process_light_events(LightFrameResources<T>& resources, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg);

    template <typename T> bool create_light_buffers(LightFrameResources<T>& resources, uint64_t required_size, orendergraph::RenderGraph& rg);
    template <typename T> void upload_all_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, orendergraph::RenderGraph& rg);
    template <typename T> void process_added_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, orendergraph::RenderGraph& rg);
    template <typename T> void process_removed_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg);
    template <typename T> void process_modified_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, orendergraph::RenderGraph& rg);

    template <typename T> void update_light_resource_set(LightFrameResources<T>& resources, orendergraph::RenderGraph& rg, orendergraph::RenderGraphNode<orendergraph::RasterContext>& node);

    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    orendergraph::ResourceHandle<ogfx::Image> m_out_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_out_image_view;

    orendergraph::ResourceHandle<ogfx::Image> m_depth_image;
    orendergraph::ResourceHandle<ogfx::ImageView> m_depth_image_view;
};

} // namespace omege

#include "geometry_feature.tpp"

#endif // GEOMETRY_RENDER_FEATURE_HPP