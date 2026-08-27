#ifndef OGFX_LOAD_STORE_OP_HPP
#define OGFX_LOAD_STORE_OP_HPP

namespace ogfx
{

enum class LoadOp
{
    Load,
    Clear,
    DontCare
};

enum class StoreOp
{
    Store,
    DontCare
};

const char* to_string(LoadOp op);
const char* to_string(StoreOp op);

} // namespace ogfx

#endif // OGFX_LOAD_STORE_OP_HPP