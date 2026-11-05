#include "geometry_generation.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;

} // namespace

void random_color(uint32_t index, float (&color)[4]) 
{
    const float hue = static_cast<float>(index % 12) / 12.0f;
    color[0] = 0.5f + 0.5f * std::sin(hue * TWO_PI);
    color[1] = 0.5f + 0.5f * std::sin((hue + 0.3333f) * TWO_PI);
    color[2] = 0.5f + 0.5f * std::sin((hue + 0.6667f) * TWO_PI);
    color[3] = 1.0f;
}

void generate_sphere(SceneGeometry& geometry, uint32_t slices, uint32_t stacks, uint32_t triangles_per_cluster)
{
    geometry.vertices.clear();
    geometry.indices.clear();
    geometry.clusters.clear();

    if (slices < 3 || stacks < 2 || triangles_per_cluster < 2)
        return;

    const uint64_t vertex_count = (static_cast<uint64_t>(slices) + 1) * (static_cast<uint64_t>(stacks) + 1);

    if (vertex_count > std::numeric_limits<uint32_t>::max())
        return;

    // Generate sphere vertices
    for (uint32_t stack = 0; stack <= stacks; ++stack)
    {
        const float v = static_cast<float>(stack) / static_cast<float>(stacks);

        const float phi = v * PI;

        for (uint32_t slice = 0; slice <= slices; ++slice)
        {
            const float u = static_cast<float>(slice) / static_cast<float>(slices);

            const float theta = u * TWO_PI;

            Vertex vertex{};

            vertex.position[0] = std::sin(phi) * std::cos(theta);
            vertex.position[1] = std::cos(phi);
            vertex.position[2] = std::sin(phi) * std::sin(theta);
            vertex.position[3] = 1.0f;

            vertex.normal[0] = vertex.position[0];
            vertex.normal[1] = vertex.position[1];
            vertex.normal[2] = vertex.position[2];
            vertex.normal[3] = 0.0f;

            geometry.vertices.push_back(vertex);
        }
    }

    // A cell produces two triangles
    const uint32_t cells_per_patch = std::max(1u, static_cast<uint32_t>(std::sqrt(static_cast<float>(triangles_per_cluster) / 2.0f)));

    const uint32_t patch_slices = cells_per_patch;
    const uint32_t patch_stacks = cells_per_patch;

    const uint32_t row_size = slices + 1;

    // Generate index ranges patch by patch.
    for (uint32_t stack_start = 0; stack_start < stacks; stack_start += patch_stacks)
    {
        const uint32_t stack_end = std::min(stack_start + patch_stacks, stacks);

        for (uint32_t slice_start = 0; slice_start < slices; slice_start += patch_slices)
        {
            const uint32_t slice_end = std::min(slice_start + patch_slices, slices);

            MeshCluster cluster{};
            cluster.first_index = static_cast<uint32_t>(geometry.indices.size());

            const uint32_t cluster_index = static_cast<uint32_t>(geometry.clusters.size());

            // Give each patch a distinct color
            random_color(cluster_index, cluster.color);

            for (uint32_t stack = stack_start; stack < stack_end; ++stack)
            {
                for (uint32_t slice = slice_start; slice < slice_end; ++slice)
                {
                    const uint32_t a = stack * row_size + slice;
                    const uint32_t b = a + row_size;
                    const uint32_t c = a + 1;
                    const uint32_t d = b + 1;

                    // Outward-facing winding
                    geometry.indices.push_back(a);
                    geometry.indices.push_back(c);
                    geometry.indices.push_back(b);

                    geometry.indices.push_back(c);
                    geometry.indices.push_back(d);
                    geometry.indices.push_back(b);
                }
            }

            cluster.index_count = static_cast<uint32_t>(geometry.indices.size()) - cluster.first_index;

            geometry.clusters.push_back(cluster);
        }
    }
}