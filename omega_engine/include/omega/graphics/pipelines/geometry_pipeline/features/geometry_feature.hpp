#ifndef GEOMTRY_RENDER_FEATURE_HPP
#define GEOMTRY_RENDER_FEATURE_HPP

#include <orenderer/render_feature.hpp>

#include <cstdint>

namespace omega
{

class EntitySystem;
class MeshSystem;
class TransformSystem;
class CameraSystem;

struct GeometryRenderFeatureDesc
{
    EntitySystem& scene;
    MeshSystem& mesh_system;
    TransformSystem& transform_system;
    CameraSystem& camera_system;

    uint32_t width = 0;
    uint32_t height = 0;
};

class GeometryRenderFeature : public orenderer::IRenderFeature
{
  public:
    GeometryRenderFeature(const GeometryRenderFeatureDesc& desc);

    void build(orenderer::RenderFeatureContext& ctx) override;

  private:
    GeometryRenderFeatureDesc m_desc;
};

}

#endif // GEOMTRY_RENDER_FEATURE_HPP