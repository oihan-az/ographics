#ifndef OGFX_PIPELINE_LAYOUT_HPP
#define OGFX_PIPELINE_LAYOUT_HPP

#include <ogfx/inline_constant_range.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class Device;
class ResourceSetLayout;

struct PipelineLayoutDesc
{
    std::vector<const ResourceSetLayout*> set_layouts{};
    std::vector<InlineConstantRange> inline_constant_ranges{};
};

class PipelineLayout
{
  public:
    PipelineLayout(const Device& device, const PipelineLayoutDesc& desc);
    ~PipelineLayout();

    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout& operator=(const PipelineLayout&) = delete;

    PipelineLayout(PipelineLayout&&) noexcept;
    PipelineLayout& operator=(PipelineLayout&&) noexcept;

  private:
    friend class GraphicsPipeline;
    friend class CommandBuffer;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_PIPELINE_LAYOUT_HPP