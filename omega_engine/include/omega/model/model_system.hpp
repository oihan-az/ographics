#ifndef OMEGA_MODEL_SYSTEM_HPP
#define OMEGA_MODEL_SYSTEM_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/assets/imported_model.hpp>
#include <omega/model/model_handle.hpp>
#include <omega/texture/texture_handle.hpp>

#include <filesystem>

namespace omega
{

class EntitySystem;
class HierarchySystem;
class ComponentSystem;
class MeshSystem;
class ModelImporter;

class ModelSystem
{
  public:
    ModelSystem(EntitySystem& entities, HierarchySystem& hierarchy, ComponentSystem& components, MeshSystem& meshes,
                ModelImporter& importer);

    ModelHandle load(const std::filesystem::path& path);
    bool unload(ModelHandle model);

    bool is_valid(ModelHandle model) const;
    bool has(ModelHandle model) const;

    const ImportedModel* get(ModelHandle model) const;

    bool instantiate(ModelHandle model, TextureHandle texture, Entity& root);

  private:
    struct ModelSlot
    {
        uint32_t generation = 0;
        bool occupied = false;

        ImportedModel model;
    };

    std::vector<ModelSlot> m_models;
    std::vector<uint32_t> m_free_indices;

    ModelImporter& m_importer;

    EntitySystem& m_entities;
    HierarchySystem& m_hierarchy;
    ComponentSystem& m_components;
    MeshSystem& m_meshes;
};

} // namespace omega

#endif