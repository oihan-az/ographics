#ifndef OGFX_TRANSFER_COMMANDS_HPP
#define OGFX_TRANSFER_COMMANDS_HPP

#include <cstdint>

namespace ogfx
{

class Buffer;
class Image;

struct BufferToImageCopyInfo
{
    const Buffer* buffer = nullptr;

    uint64_t buffer_offset = 0;
    uint32_t buffer_row_length = 0;
    uint32_t buffer_image_height = 0;

    const Image* image = nullptr;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;

    uint32_t mip_level = 0;
    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;
};

struct GenerateMipmapsInfo
{
    const Image* image = nullptr;
};

} // namespace ogfx

#endif // OGFX_TRANSFER_COMMANDS_HPP