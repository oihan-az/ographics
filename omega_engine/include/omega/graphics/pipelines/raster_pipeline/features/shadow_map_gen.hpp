#ifndef OMEGA_SHADOW_MAP_GEN_HPP
#define OMEGA_SHADOW_MAP_GEN_HPP

#include <orenderer/render_feature.hpp>
#include <orendergraph/resource_handle.hpp>

#include <omega/core/entity/entity.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/frames_in_flight/frame_event_queue.hpp>

#include <omega/shadows/directional_shadow.hpp>
#include <omega/shadows/point_shadow.hpp>
#include <omega/shadows/spot_shadow.hpp>
#include <omega/shadows/shadow_caster.hpp>

#include <omega/lighting/directional_light.hpp>
#include <omega/lighting/point_light.hpp>
#include <omega/lighting/spot_light.hpp>

#include <omega/graphics/resources/gpu_shadow_data.hpp>

#include <cstdint>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

namespace ogfx
{
class Image;
class ImageView;
class CommandBuffer;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
struct RasterContext;
template <typename TContext> class RenderGraphNode;
} // namespace orendergraph

namespace omega
{

class ComponentSystem;
class EventSystem;
class TransformSystem;
class GraphicsSystem;

enum class ShadowMapType : uint8_t
{
    Directional,
    Spot,
    Point
};

struct ShadowMapGenDesc
{
    Entity camera;

    ComponentSystem& components;
    EventSystem& events;
    TransformSystem& transforms;
    GraphicsSystem& graphics;

    ShadowProjectionSettings proj_setting{};
};

class ShadowMapGen : public orenderer::IRenderFeature
{
public:
    explicit ShadowMapGen(const ShadowMapGenDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;
    void set_camera(Entity camera) { m_desc.camera = camera; }

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* node() const
    {
        return m_shadow_node;
    }

    orendergraph::ResourceHandle<ogfx::Image> image(ShadowMapType type, Entity light) const;
    orendergraph::ResourceHandle<ogfx::ImageView> sampled_view(ShadowMapType type, Entity light) const;
    orendergraph::ResourceHandle<ogfx::ImageView> fallback_view(ShadowMapType type) const;

    ShadowProjectionSettings projection_settings() const;

private:
    struct ShadowMap
    {
        uint32_t resolution = 0;
        orendergraph::ResourceHandle<ogfx::Image> image;
        orendergraph::ResourceHandle<ogfx::ImageView> sampled_view;
        std::vector<orendergraph::ResourceHandle<ogfx::ImageView>> attachment_views;
        std::vector<uint32_t> framebuffer_indices;

        bool was_rendered = false;
    };

    using ShadowRegistry = std::unordered_map<Entity, ShadowMap, EntityHash>;

    struct RetiredShadowMap
    {
        ShadowMap map;
        uint32_t frames_remaining = 0;
    };

    ShadowMapGenDesc m_desc;

    FrameEventQueue<ComponentEvent<DirectionalShadow>> m_directional_events;
    FrameEventQueue<ComponentEvent<SpotShadow>> m_spot_events;
    FrameEventQueue<ComponentEvent<PointShadow>> m_point_events;

    ShadowRegistry m_directional_maps;
    ShadowRegistry m_spot_maps;
    ShadowRegistry m_point_maps;
    std::deque<RetiredShadowMap> m_retired_maps;

    ShadowMap m_fallback_2d;
    ShadowMap m_fallback_cube;

    void initialize_fallback_maps(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd);

    orendergraph::RenderGraphNode<orendergraph::RasterContext>* m_shadow_node = nullptr;

    void add_node(orendergraph::RenderGraph& rg);
    void create_existing_maps(orendergraph::RenderGraph& rg);

    ShadowMap create_shadow_map(orendergraph::RenderGraph& rg, uint32_t resolution, bool cubemap);
    void upsert_shadow_map(orendergraph::RenderGraph& rg, ShadowRegistry& registry, Entity light, uint32_t resolution, bool cubemap);
    void remove_shadow_map(ShadowRegistry& registry, Entity light);
    void retire_shadow_map(ShadowMap&& map);
    void collect_retired_maps(orendergraph::RenderGraph& rg);
    void destroy_shadow_map(orendergraph::RenderGraph& rg, ShadowMap& map);

    template <typename TShadow>
    void process_shadow_events(orendergraph::RenderGraph& rg,
                               FrameEventQueue<ComponentEvent<TShadow>>& queue,
                               ShadowRegistry& registry,
                               uint32_t min_resolution, uint32_t max_resolution,
                               bool cubemap);

    void process_events(orendergraph::RenderGraph& rg);
    void transition_all(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, bool to_depth_attachment);
    void execute_shadow_passes(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd);
    void execute_registry(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, const ShadowRegistry& registry, ShadowMapType type);
    void draw_shadow_casters(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, ShadowMapType type, Entity light, uint32_t face);

    const ShadowRegistry& shadow_registry(ShadowMapType type) const;
};

} // namespace omega

#endif // OMEGA_SHADOW_MAP_GEN_HPP
