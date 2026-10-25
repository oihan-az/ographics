#ifndef OGFX_BARRIER_COMMANDS_HPP
#define OGFX_BARRIER_COMMANDS_HPP

#include <ogfx/pipeline_stage.hpp>
#include <ogfx/access_flags.hpp>
#include <ogfx/image_state.hpp>

#include <cstdint>

namespace ogfx
{

class Buffer;
class Image;

struct MemoryBarrierInfo
{
    PipelineStage src_stages = PipelineStage::None;
    PipelineStage dst_stages = PipelineStage::None;

    AccessFlags src_access = AccessFlags::None;
    AccessFlags dst_access = AccessFlags::None;
};

struct BufferBarrierInfo
{
    const Buffer* buffer = nullptr;

    uint64_t offset = 0;
    uint64_t size = 0;

    MemoryBarrierInfo memory_barrier_info;
};

struct ImageBarrierInfo
{
    const Image* image = nullptr;

    ImageState old_state = ImageState::Undefined;
    ImageState new_state = ImageState::Undefined;

    uint32_t base_mip_level = 0;
    uint32_t mip_level_count = 1;
    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;

    MemoryBarrierInfo memory_barrier_info;
};

struct ImageTransitionInfo
{
    const Image* image = nullptr;

    ImageState old_state = ImageState::Undefined;
    ImageState new_state = ImageState::Undefined;

    uint32_t base_mip_level = 0;
    uint32_t mip_level_count = 1;

    uint32_t base_array_layer = 0;
    uint32_t array_layer_count = 1;
};

} // namespace ogfx

#endif // OGFX_BARRIER_COMMANDS_HPP