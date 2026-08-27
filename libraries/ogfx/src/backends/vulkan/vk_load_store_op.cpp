#include "vk_load_store_op.hpp"

namespace ogfx
{

VkAttachmentLoadOp to_vk_load_op(LoadOp op)
{
    switch (op)
    {
    case LoadOp::Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;

    case LoadOp::Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;

    case LoadOp::DontCare:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    default:
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

LoadOp from_vk_load_op(VkAttachmentLoadOp op)
{
    switch (op)
    {
    case VK_ATTACHMENT_LOAD_OP_LOAD:
        return LoadOp::Load;

    case VK_ATTACHMENT_LOAD_OP_CLEAR:
        return LoadOp::Clear;

    case VK_ATTACHMENT_LOAD_OP_DONT_CARE:
        return LoadOp::DontCare;

    default:
        return LoadOp::DontCare;
    }
}

VkAttachmentStoreOp to_vk_store_op(StoreOp op)
{
    switch (op)
    {
    case StoreOp::Store:
        return VK_ATTACHMENT_STORE_OP_STORE;

    case StoreOp::DontCare:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;

    default:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }
}

StoreOp from_vk_store_op(VkAttachmentStoreOp op)
{
    switch (op)
    {
    case VK_ATTACHMENT_STORE_OP_STORE:
        return StoreOp::Store;

    case VK_ATTACHMENT_STORE_OP_DONT_CARE:
        return StoreOp::DontCare;

    default:
        return StoreOp::DontCare;
    }
}

} // namespace ogfx