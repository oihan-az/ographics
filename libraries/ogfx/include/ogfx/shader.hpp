#ifndef OGFX_SHADER_HPP
#define OGFX_SHADER_HPP

#include <ogfx/shader_stage.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace ogfx
{

class Device;

struct ShaderDesc
{
    ShaderStage stage = ShaderStage::Vertex;
    std::vector<uint8_t> bytecode;
};

class Shader
{
  public:
    Shader(const Device& device, const ShaderDesc& desc);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&&) noexcept;

  private:

    ShaderStage m_stage;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_SHADER_HPP