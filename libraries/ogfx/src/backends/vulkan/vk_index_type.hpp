#ifndef OGFX_VK_INDEX_TYPE_HPP
#define OGFX_VK_INDEX_TYPE_HPP

#include <ogfx/index_type.hpp>
#include <vulkan/vulkan.h>

namespace ogfx
{

VkIndexType to_vk_index_type(IndexType index_type);
IndexType from_vk_index_type(VkIndexType index_type);

} // namespace ogfx

#endif // OGFX_VK_INDEX_TYPE_HPP