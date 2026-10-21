#ifndef OGFX_SAMPLER_HPP
#define OGFX_SAMPLER_HPP

#include <memory>

namespace ogfx
{

class Device;

enum class Filter
{
    Undefined,
    Nearest,
    Linear
};

enum class MipmapFilter
{
    Undefined,
    Nearest,
    Linear
};

enum class AddressMode
{
    Undefined,
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

enum class BorderColor
{
    Undefined,
    TransparentBlack,
    OpaqueBlack,
    OpaqueWhite
};

struct SamplerDesc
{
    Filter min_filter = Filter::Linear;
    Filter mag_filter = Filter::Linear;
    MipmapFilter mip_filter = MipmapFilter::Linear;

    AddressMode address_u = AddressMode::Repeat;
    AddressMode address_v = AddressMode::Repeat;
    AddressMode address_w = AddressMode::Repeat;

    BorderColor border_color = BorderColor::OpaqueBlack;

    float mip_lod_bias = 0.0f;
    float min_lod = 0.0f;
    float max_lod = 0.0f;
    bool anisotropy_enable = false;
    float max_anisotropy = 1.0f;
};

class Sampler
{
  public:
    Sampler(const Device& device, const SamplerDesc& desc);
    ~Sampler();

    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;

    Sampler(Sampler&&) noexcept;
    Sampler& operator=(Sampler&&) noexcept;

  private:
    friend class ResourceSet;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_SAMPLER_HPP