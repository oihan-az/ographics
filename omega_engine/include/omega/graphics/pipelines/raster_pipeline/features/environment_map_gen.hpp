#ifndef OMEGA_ENVIRONMENT_MAP_GEN_HPP
#define OMEGA_ENVIRONMENT_MAP_GEN_HPP

#include <orenderer/render_feature.hpp>

#include <orendergraph/resource_handle.hpp>

#include <omega/core/entity/entity.hpp>

namespace ogfx
{
class Image;
class ImageView;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
template <typename TContext> class RenderGraphNode;
struct ComputeContext;
} // namespace orendergraph

namespace omega
{

class ComponentSystem;
class TransformSystem;
class GraphicsSystem;

struct EnvironmentMapGenDesc
{
    Entity camera;
    Entity atmosphere;

    ComponentSystem& components;
    TransformSystem& transforms;
    GraphicsSystem& graphics;
};

class EnvironmentMapGen : public orenderer::IRenderFeature
{
  public:
    explicit EnvironmentMapGen(const EnvironmentMapGenDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

    void set_camera(Entity camera);
    void set_atmosphere(Entity atmosphere);

    orendergraph::ResourceHandle<ogfx::Image> environment_map() const
    {
        return m_environment_map;
    }

    orendergraph::ResourceHandle<ogfx::ImageView> environment_map_view() const
    {
        return m_environment_map_view;
    }

  private:
    void create_resources(orendergraph::RenderGraph& rg);
    void add_node(orendergraph::RenderGraph& rg);

    EnvironmentMapGenDesc m_desc;

    orendergraph::ResourceHandle<ogfx::Image> m_environment_map;
    orendergraph::ResourceHandle<ogfx::ImageView> m_environment_map_storage_view;
    orendergraph::ResourceHandle<ogfx::ImageView> m_environment_map_view;

    orendergraph::RenderGraphNode<orendergraph::ComputeContext>* m_environment_node = nullptr;
};

} // namespace omega

#endif // OMEGA_ENVIRONMENT_MAP_GEN_HPP