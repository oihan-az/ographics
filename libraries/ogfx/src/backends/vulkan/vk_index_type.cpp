#include "vk_index_type.hpp"

namespace ogfx
{

VkIndexType to_vk_index_type(IndexType index_type)
{
    switch (index_type)
    {
    case IndexType::UInt16:
        return VK_INDEX_TYPE_UINT16;

    case IndexType::UInt32:
        return VK_INDEX_TYPE_UINT32;

    case IndexType::Undefined:
    default:
        return VK_INDEX_TYPE_MAX_ENUM;
    }
}

IndexType from_vk_index_type(VkIndexType index_type)
{
    switch (index_type)
    {
    case VK_INDEX_TYPE_UINT16:
        return IndexType::UInt16;

    case VK_INDEX_TYPE_UINT32:
        return IndexType::UInt32;

    default:
        return IndexType::Undefined;
    }
}

} // namespace ogfx