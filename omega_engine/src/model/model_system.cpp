#include <omega/model/model_system.hpp>

#include <omega/core/entity/entity_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/mesh/mesh_system.hpp>
#include <omega/renderable/renderable.hpp>
#include <omega/transform/transform.hpp>
#include <omega/assets/model_importer.hpp>

#include <vector>
#include <iostream>

namespace omega
{

ModelSystem::ModelSystem(EntitySystem& entities, HierarchySystem& hierarchy, ComponentSystem& components,
                         MeshSystem& meshes, ModelImporter& importer)
    : m_entities(entities), m_hierarchy(hierarchy), m_components(components), m_meshes(meshes), m_importer(importer)
{
}

ModelHandle ModelSystem::load(const std::filesystem::path& path)
{
    omega::ImportedModel model = m_importer.import(path);

    uint32_t index = 0;
    if (!m_free_indices.empty())
    {
        index = m_free_indices.back();
        m_free_indices.pop_back();
    }
    else
    {
        index = static_cast<uint32_t>(m_models.size());
        m_models.emplace_back();
    }

    ModelSlot& slot = m_models[index];

    slot.model = std::move(model);
    slot.occupied = true;

    return ModelHandle{index, slot.generation};
}

bool ModelSystem::unload(ModelHandle model)
{
    if (!is_valid(model))
        return false;

    ModelSlot& slot = m_models[model.index];

    slot.model = ImportedModel{};
    slot.occupied = false;
    ++slot.generation;

    m_free_indices.push_back(model.index);

    return true;
}

bool ModelSystem::is_valid(ModelHandle model) const
{
    return model.is_valid() && model.index < m_models.size() && m_models[model.index].occupied &&
           m_models[model.index].generation == model.generation;
}

bool ModelSystem::has(ModelHandle model) const
{
    return is_valid(model);
}

const ImportedModel* ModelSystem::get(ModelHandle model) const
{
    if (!is_valid(model))
        return nullptr;

    return &m_models[model.index].model;
}

bool ModelSystem::instantiate(ModelHandle model, Entity& root)
{
    const ImportedModel* imported_model = get(model);
    if (!imported_model)
        return false;

    root = Entity{};

    if (imported_model->nodes.empty() || imported_model->root_nodes.empty())
        return false;

    // Validate node references before creating anything
    for (const ImportedNode& node : imported_model->nodes)
    {
        for (uint32_t mesh_index : node.meshes)
        {
            if (mesh_index >= imported_model->meshes.size())
                return false;
        }

        for (uint32_t child_index : node.children)
        {
            if (child_index >= imported_model->nodes.size())
                return false;
        }
    }

    // Create each imported mesh only once
    std::vector<std::vector<MeshHandle>> mesh_handles(imported_model->meshes.size());
    std::vector<MeshHandle> created_meshes;
    for (uint32_t i = 0; i < imported_model->meshes.size(); ++i)
    {
        const ImportedMesh& imported_mesh = imported_model->meshes[i];
        mesh_handles[i].reserve(imported_mesh.primitives.size());

        for (const ImportedPrimitive& primitive : imported_mesh.primitives)
        {
            MeshHandle handle{};

            if (!m_meshes.create_mesh(primitive.mesh, handle))
            {
                for (MeshHandle created : created_meshes)
                    m_meshes.release_mesh(created);

                return false;
            }

            mesh_handles[i].push_back(handle);
            created_meshes.push_back(handle);
        }
    }

    std::vector<Entity> node_entities(imported_model->nodes.size());
    std::vector<Entity> created_entities;

    auto rollback = [&]()
    {
        for (auto it = created_entities.rbegin(); it != created_entities.rend(); ++it)
        {
            m_entities.destroy_entity(*it);
        }

        for (MeshHandle handle : created_meshes)
            m_meshes.release_mesh(handle);
    };

    // Create node entities and their transforms
    for (uint32_t i = 0; i < imported_model->nodes.size(); ++i)
    {
        const Entity entity = m_entities.create_entity();

        if (!entity.is_valid())
        {
            rollback();
            return false;
        }

        node_entities[i] = entity;
        created_entities.push_back(entity);

        if (!m_components.add<Transform>(entity, imported_model->nodes[i].local_transform))
        {
            rollback();
            return false;
        }
    }

    // Create a renderable child entity for every primitive
    for (uint32_t i = 0; i < imported_model->nodes.size(); ++i)
    {
        const ImportedNode& node = imported_model->nodes[i];
        const Entity parent = node_entities[i];

        for (uint32_t mesh_index : node.meshes)
        {
            const auto& handles = mesh_handles[mesh_index];

            for (MeshHandle handle : handles)
            {
                const Entity renderable_entity = m_entities.create_entity();

                if (!renderable_entity.is_valid())
                {
                    rollback();
                    return false;
                }

                created_entities.push_back(renderable_entity);

                if (!m_components.add<Transform>(renderable_entity, Transform{}))
                {
                    rollback();
                    return false;
                }

                Renderable renderable{};
                renderable.mesh = handle;

                if (!m_components.add<Renderable>(renderable_entity, renderable))
                {
                    rollback();
                    return false;
                }

                if (!m_hierarchy.set_parent(renderable_entity, parent))
                {
                    rollback();
                    return false;
                }
            }
        }
    }

    // Reconstruct the imported node hierarchy
    for (uint32_t i = 0; i < imported_model->nodes.size(); ++i)
    {
        for (uint32_t child_index : imported_model->nodes[i].children)
        {
            if (!m_hierarchy.set_parent(node_entities[child_index], node_entities[i]))
            {
                rollback();
                return false;
            }
        }
    }

    // Resolve the root entity
    if (imported_model->root_nodes.size() != 1)
    {
        rollback();
        return false;
    }

    const uint32_t root_index = imported_model->root_nodes.front();
    if (root_index >= node_entities.size())
    {
        rollback();
        return false;
    }

    root = node_entities[root_index];

    return true;
}

} // namespace omega