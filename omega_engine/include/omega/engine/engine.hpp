#ifndef OMEGA_ENGINE_HPP
#define OMEGA_ENGINE_HPP

namespace omega
{

class Engine
{
  public:
    Engine(const EngineDesc& desc);
    ~Engine();

    void update(float delta_time);

    EntitySystem& entities();
    HierarchySystem& hierarchy();
    ComponentSystem& components();

    TransformSystem& transforms();
    CameraSystem& cameras();

    MeshSystem& meshes();
    TextureSystem& textures();
    ModelSystem& models();

    GpuMeshSystem& gpu_meshes();
    GpuTextureSystem& gpu_textures();

    orenderer::Renderer& renderer();

  private:
    EventSystem m_events;
    EntitySystem m_entities;
    HierarchySystem m_hierarchy;
    ComponentSystem m_components;
    TransformSystem m_transforms;
    CameraSystem m_cameras;

    GraphicsSystem m_graphics;

    omega::ModelImporter m_importer;
    ModelSystem m_models;

    orenderer::Renderer m_renderer;
};

}

#endif // OMEGA_ENGINE_HPP