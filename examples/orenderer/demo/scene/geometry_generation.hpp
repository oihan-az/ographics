#ifndef GEOMETRY_GENERATION_HPP
#define GEOMETRY_GENERATION_HPP

#include <cstdint>
#include <vector>

struct Vertex
{
    float position[4]{};
    float normal[4]{};
};

using Index = uint32_t;

struct MeshCluster
{
    float color[4]{1.0f, 1.0f, 1.0f, 1.0f};

    uint32_t first_index = 0;
    uint32_t index_count = 0;
    uint32_t padding[2]{};
};

struct SceneGeometry
{
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<MeshCluster> clusters;
};

void generate_sphere(SceneGeometry& geometry, uint32_t slices, uint32_t stacks, uint32_t triangles_per_cluster);

#endif // GEOMETRY_GENERATION_HPP