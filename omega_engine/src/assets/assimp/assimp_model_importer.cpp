#include <omega/assets/model_importer.hpp>

#include <omega/assets/imported_model.hpp>
#include <omega/assets/texture_loader.hpp>
#include <omega/mesh/vertex.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

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

ImportedMesh convert_mesh(const aiMesh& source, uint32_t albedo_texture)
{
    if (!source.HasPositions() || source.mNumVertices == 0)
        throw std::runtime_error("Assimp mesh has no vertex positions.");

    ImportedMesh result;
    result.name = source.mName.C_Str();

    ImportedPrimitive primitive;

    primitive.albedo_texture = albedo_texture;

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

uint32_t load_material_albedo(const aiScene& scene, const aiMaterial& material, const std::filesystem::path& directory,
                              ImportedModel& model)
{
    aiString texture_path;
    aiReturn status = material.GetTexture(aiTextureType_BASE_COLOR, 0, &texture_path);
    if (status != AI_SUCCESS)
    {
        status = material.GetTexture(aiTextureType_DIFFUSE, 0, &texture_path);
    }

    if (status != AI_SUCCESS)
        return invalid_texture_index;

    TextureAsset texture;
    const aiTexture* embedded = scene.GetEmbeddedTexture(texture_path.C_Str());
    if (embedded)
    {
        // TODO: support embedded textures
        return invalid_texture_index;
    }
    else
    {
        std::filesystem::path file_path(texture_path.C_Str());

        if (file_path.is_relative())
            file_path = directory / file_path;

        if (!TextureLoader::load(file_path, texture))
        {
            return invalid_texture_index;
        }
    }

    if (model.textures.size() >= invalid_texture_index)
        return invalid_texture_index;

    const uint32_t texture_index = static_cast<uint32_t>(model.textures.size());
    model.textures.push_back(ImportedTexture{.texture = std::move(texture)});
    return texture_index;
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

    // Load material textures
    const std::filesystem::path directory = path.parent_path();
    std::vector<uint32_t> material_textures(scene->mNumMaterials, invalid_texture_index);
    for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
    {
        material_textures[i] = load_material_albedo(*scene, *scene->mMaterials[i], directory, result);
    }

    // Convert meshes
    result.meshes.reserve(scene->mNumMeshes);
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh& mesh = *scene->mMeshes[i];

        uint32_t albedo_texture = invalid_texture_index;

        if (mesh.mMaterialIndex < material_textures.size())
            albedo_texture = material_textures[mesh.mMaterialIndex];

        result.meshes.push_back(convert_mesh(mesh, albedo_texture));
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