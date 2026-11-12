#include <omega/assets/model_importer.hpp>

#include <omega/assets/imported_model.hpp>
#include <omega/mesh/vertex.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

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

    // Convert meshes
    // Nodes will reference these by index
    result.meshes.reserve(scene->mNumMeshes);
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        result.meshes.push_back(convert_mesh(*scene->mMeshes[i]));
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