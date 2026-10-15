#ifndef OGFX_INPUT_ASSEMBLY_HPP
#define OGFX_INPUT_ASSEMBLY_HPP

namespace ogfx
{

enum class PrimitiveTopology
{
    Undefined,
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    LineListWithAdjacency,
    LineStripWithAdjacency,
    TriangleListWithAdjacency,
    TriangleStripWithAdjacency,
    PatchList
};

const char* to_string(PrimitiveTopology topology);

struct InputAssemblyStateDesc
{
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    bool primitive_restart_enable = false;
};

} // namespace ogfx

#endif // OGFX_INPUT_ASSEMBLY_HPP