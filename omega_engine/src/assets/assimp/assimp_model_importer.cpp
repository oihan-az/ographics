#include <omega/assets/model_importer.hpp>

#include <omega/assets/imported_model.hpp>
#include <omega/assets/texture_loader.hpp>
#include <omega/mesh/vertex.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

#include <map>
#include <utility>

namespace omega
{

namespace
{

Vector3 to_vector3(const aiVector3D& value)
{
    return Vector3{value.x, value.y, value.z};
}

Vector2 to_vector2(const aiVector3D& value)
{
    return Vector2{value.x, value.y};
}

float vector_length(const Vector3& value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

Vector3 normalize(const Vector3& value)
{
    const float length = vector_length(value);

    if (length <= 0.000001f)
        return Vector3{0.0f, 0.0f, 1.0f};

    return value / length;
}

Vector3 cross(const Vector3& a, const Vector3& b)
{
    return Vector3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

void build_fallback_tangent_space(const Vector3& normal, Vector3& tangent)
{
    const Vector3 reference = std::abs(normal.z) < 0.999f ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{0.0f, 1.0f, 0.0f};

    tangent = normalize(cross(reference, normal));
}

Transform convert_transform(const aiMatrix4x4& matrix)
{
    aiVector3D position;
    aiVector3D scale;
    aiQuaternion rotation;

    matrix.Decompose(scale, rotation, position);

    Transform result;
    result.position = to_vector3(position);
    result.rotation = Quaternion{rotation.w, rotation.x, rotation.y, rotation.z};
    result.scale = to_vector3(scale);

    return result;
}

ImportedMesh convert_mesh(const aiMesh& source)
{
    if (!source.HasPositions() || source.mNumVertices == 0)
        throw std::runtime_error("Assimp mesh has no vertex positions.");

    ImportedMesh result;
    result.name = source.mName.C_Str();

    ImportedPrimitive primitive;

    primitive.material = source.mMaterialIndex;

    MeshAsset& asset = primitive.mesh;

    asset.vertex_input = standard_vertex_layout();
    asset.vertex_count = source.mNumVertices;

    std::vector<Vertex> vertices(source.mNumVertices);

    const bool has_normals = source.HasNormals();
    const bool has_uvs = source.HasTextureCoords(0);
    const bool has_tangent_space = source.HasTangentsAndBitangents();

    for (uint32_t i = 0; i < source.mNumVertices; ++i)
    {
        Vertex& vertex = vertices[i];

        vertex.position = to_vector3(source.mVertices[i]);
        vertex.normal = has_normals ? normalize(to_vector3(source.mNormals[i])) : Vector3{0.0f, 0.0f, 1.0f};
        vertex.uv = has_uvs ? to_vector2(source.mTextureCoords[0][i]) : Vector2{0.0f};

        if (has_tangent_space)
        {
            vertex.tangent = normalize(to_vector3(source.mTangents[i]));
        }
        else
        {
            build_fallback_tangent_space(vertex.normal, vertex.tangent);
        }
    }

    asset.vertex_data.emplace_back(sizeof(Vertex) * vertices.size());

    std::memcpy(asset.vertex_data[0].data(), vertices.data(), asset.vertex_data[0].size());

    for (uint32_t i = 0; i < source.mNumFaces; ++i)
    {
        const aiFace& face = source.mFaces[i];

        if (face.mNumIndices != 3)
            throw std::runtime_error("Assimp mesh contains a non-triangle face.");

        for (uint32_t j = 0; j < 3; ++j)
            asset.indices.push_back(face.mIndices[j]);
    }

    result.primitives.push_back(std::move(primitive));
    return result;
}

constexpr uint32_t invalid_texture_index = std::numeric_limits<uint32_t>::max();


enum class MaterialTextureColorSpace
{
    Linear,
    SRGB
};

class TextureImportRegistry
{
  public:
    TextureImportRegistry(const aiScene& scene, const std::filesystem::path& directory, ImportedModel& model)
        : m_scene(scene), m_directory(directory), m_model(model)
    {
    }

    uint32_t load(const aiMaterial& material, aiTextureType type, MaterialTextureColorSpace color_space)
    {
        const auto path = get_texture_path(material, type);

        if (path.empty())
            return invalid_imported_index;

        const TextureKey key{path, color_space};

        if (const auto it = m_indices.find(key); it != m_indices.end())
            return it->second;

        // Embedded textures are not supported yet
        aiString source_path;

        if (material.GetTexture(type, 0, &source_path) != AI_SUCCESS)
            return invalid_imported_index;

        if (m_scene.GetEmbeddedTexture(source_path.C_Str()))
            return invalid_imported_index;

        TextureAsset texture;

        if (!TextureLoader::load(path, texture))
            return invalid_imported_index;

        if (color_space == MaterialTextureColorSpace::SRGB)
        {
            switch (texture.format)
            {
            case ogfx::Format::RGBA8_UNORM:
                texture.format = ogfx::Format::RGBA8_SRGB;
                break;

            default:
                break;
            }
        }

        if (m_model.textures.size() >= invalid_imported_index)
            return invalid_imported_index;

        const uint32_t index = static_cast<uint32_t>(m_model.textures.size());

        m_model.textures.push_back(ImportedTexture{.texture = std::move(texture)});

        m_indices.emplace(key, index);

        return index;
    }

    uint32_t find(const aiMaterial& material, aiTextureType type, MaterialTextureColorSpace color_space) const
    {
        const auto path = get_texture_path(material, type);

        if (path.empty())
            return invalid_imported_index;

        const TextureKey key{path, color_space};

        const auto it = m_indices.find(key);

        return it != m_indices.end() ? it->second : invalid_imported_index;
    }

  private:
    using TextureKey = std::pair<std::filesystem::path, MaterialTextureColorSpace>;

    std::filesystem::path get_texture_path(const aiMaterial& material, aiTextureType type) const
    {
        aiString source;

        if (material.GetTexture(type, 0, &source) != AI_SUCCESS)
            return {};

        std::filesystem::path path(source.C_Str());

        if (path.is_relative())
            path = m_directory / path;

        return path.lexically_normal();
    }

    const aiScene& m_scene;
    const std::filesystem::path m_directory;
    ImportedModel& m_model;

    std::map<TextureKey, uint32_t> m_indices;
};

void load_material_textures(const aiMaterial& material, TextureImportRegistry& textures)
{
    // Base color, with legacy diffuse fallback
    const uint32_t base_color = textures.load(material, aiTextureType_BASE_COLOR, MaterialTextureColorSpace::SRGB);
    if (base_color == invalid_imported_index)
    {
        textures.load(material, aiTextureType_DIFFUSE, MaterialTextureColorSpace::SRGB);
    }

    textures.load(material, aiTextureType_METALNESS, MaterialTextureColorSpace::Linear);

    textures.load(material, aiTextureType_NORMALS, MaterialTextureColorSpace::Linear);

    textures.load(material, aiTextureType_LIGHTMAP, MaterialTextureColorSpace::Linear);

    textures.load(material, aiTextureType_EMISSIVE, MaterialTextureColorSpace::SRGB);
}

ImportedPbrOpaqueMaterial convert_material(const aiMaterial& source, const TextureImportRegistry& textures)
{
    ImportedPbrOpaqueMaterial result;

    aiColor4D base_color{1.0f, 1.0f, 1.0f, 1.0f};

    if (source.Get(AI_MATKEY_BASE_COLOR, base_color) == AI_SUCCESS)
    {
        result.base_color_factor = {base_color.r, base_color.g, base_color.b, base_color.a};
    }

    float metallic = 1.0f;
    if (source.Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
        result.metallic_factor = metallic;

    float roughness = 1.0f;
    if (source.Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
        result.roughness_factor = roughness;

    aiColor3D emissive{0.0f, 0.0f, 0.0f};
    if (source.Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
    {
        result.emissive_factor = {emissive.r, emissive.g, emissive.b};
    }

    result.base_color_texture = textures.find(source, aiTextureType_BASE_COLOR, MaterialTextureColorSpace::SRGB);
    if (result.base_color_texture == invalid_imported_index)
    {
        result.base_color_texture = textures.find(source, aiTextureType_DIFFUSE, MaterialTextureColorSpace::SRGB);
    }

    result.metallic_roughness_texture = textures.find(source, aiTextureType_METALNESS, MaterialTextureColorSpace::Linear);

    result.normal_texture = textures.find(source, aiTextureType_NORMALS, MaterialTextureColorSpace::Linear);

    result.occlusion_texture = textures.find(source, aiTextureType_LIGHTMAP, MaterialTextureColorSpace::Linear);

    result.emissive_texture = textures.find(source, aiTextureType_EMISSIVE, MaterialTextureColorSpace::SRGB);

    return result;
}

} // namespace

ImportedModel ModelImporter::import(const std::filesystem::path& path)
{
    Assimp::Importer importer;

    constexpr unsigned int flags = aiProcess_Triangulate | 
                                   aiProcess_GenSmoothNormals | 
                                   aiProcess_CalcTangentSpace |
                                   aiProcess_ValidateDataStructure;

    const aiScene* scene = importer.ReadFile(path.string(), flags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        throw std::runtime_error("Failed to import model '" + path.string() + "': " + importer.GetErrorString());
    }

    ImportedModel result;
    result.name = path.stem().string();

    // Load unique texture assets
    TextureImportRegistry textures(*scene, path.parent_path(), result);
    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        load_material_textures(*scene->mMaterials[i], textures);
    }

    // Convert materials using imported texture indices
    result.materials.reserve(scene->mNumMaterials);
    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        result.materials.push_back(convert_material(*scene->mMaterials[i], textures));
    }

    // Convert meshes
    result.meshes.reserve(scene->mNumMeshes);
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh& mesh = *scene->mMeshes[i];
        result.meshes.push_back(convert_mesh(mesh));
    }

    // Flatten the Assimp node tree into an indexed node array
    std::function<uint32_t(const aiNode&)> convert_node;
    convert_node = [&](const aiNode& source) -> uint32_t
    {
        if (result.nodes.size() >= std::numeric_limits<uint32_t>::max())
        {
            throw std::runtime_error("Imported model contains too many nodes");
        }

        const uint32_t node_index = static_cast<uint32_t>(result.nodes.size());

        ImportedNode node;
        node.name = source.mName.C_Str();
        node.local_transform = convert_transform(source.mTransformation);

        for (uint32_t i = 0; i < source.mNumMeshes; ++i)
        {
            const uint32_t mesh_index = source.mMeshes[i];
            if (mesh_index >= result.meshes.size())
                throw std::runtime_error("Assimp node references an invalid mesh");

            node.meshes.push_back(mesh_index);
        }

        result.nodes.push_back(std::move(node));

        for (uint32_t i = 0; i < source.mNumChildren; ++i)
        {
            const uint32_t child_index = convert_node(*source.mChildren[i]);

            result.nodes[node_index].children.push_back(child_index);
        }

        return node_index;
    };

    result.root_nodes.push_back(convert_node(*scene->mRootNode));

    return result;
}

} // namespace omega