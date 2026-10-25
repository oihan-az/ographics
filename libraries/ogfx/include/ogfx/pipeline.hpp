#ifndef OGFX_PIPELINE_HPP
#define OGFX_PIPELINE_HPP

#include <ogfx/pipeline_type.hpp>

#include <memory>

namespace ogfx
{

class Device;
class PipelineLayout;

class Pipeline
{
  public:
    virtual ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline(Pipeline&&) noexcept;
    Pipeline& operator=(Pipeline&&) noexcept;

    virtual PipelineType type() const noexcept = 0;

  protected:
    friend class CommandBuffer;

    Pipeline(const Device& device);

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_PIPELINE_HPP