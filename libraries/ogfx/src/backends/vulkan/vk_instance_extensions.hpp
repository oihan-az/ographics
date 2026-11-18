#ifndef OGFX_VK_INSTANCE_EXTENSIONS_HPP
#define OGFX_VK_INSTANCE_EXTENSIONS_HPP

#include <ogfx/instance_extensions.hpp>

#include <vulkan/vulkan.h>

#include <vector>

namespace ogfx
{

std::vector<const char*> to_vk_instance_extensions(InstanceExtension extensions);

} // namespace ogfx

#endif // OGFX_VK_INSTANCE_EXTENSIONS_HPP