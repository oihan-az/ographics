#ifndef OGFX_RESOURCE_SET_LAYOUT_HPP
#define OGFX_RESOURCE_SET_LAYOUT_HPP

#include <ogfx/resource_type.hpp>
#include <ogfx/shader_stage.hpp>

#include <memory>

namespace ogfx
{

class Device;

struct ResourceBindingDesc
{
    uint32_t binding = 0;
    ResourceType type = ResourceType::Undefined;
    uint32_t array_count = 1;
    ShaderStageFlags stages = ShaderStageFlags::None;
};

struct ResourceSetLayoutDesc
{
    std::vector<ResourceBindingDesc> bindings;
};

class ResourceSetLayout
{
  public:
    ResourceSetLayout(const Device& device, const ResourceSetLayoutDesc& desc);
    ~ResourceSetLayout();

    ResourceSetLayout(const ResourceSetLayout&) = delete;
    ResourceSetLayout& operator=(const ResourceSetLayout&) = delete;

    ResourceSetLayout(ResourceSetLayout&&) noexcept;
    ResourceSetLayout& operator=(ResourceSetLayout&&) noexcept;

  private:
    friend class ResourceAllocator;
    friend class ResourceSet;
    friend class PipelineLayout;

    ResourceSetLayoutDesc m_desc{};

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_RESOURCE_SET_LAYOUT_HPP
