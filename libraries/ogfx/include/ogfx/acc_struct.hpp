#ifndef OGFX_ACCELERATION_STRUCTURE_HPP
#define OGFX_ACCELERATION_STRUCTURE_HPP

#include <ogfx/acc_struct_type.hpp>
#include <ogfx/acc_struct_geometry.hpp>
#include <ogfx/acc_struct_instance.hpp>
#include <ogfx/acc_struct_build_preference.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace ogfx
{

class PhysicalDevice;
class Device;

struct AccelerationStructureDesc
{
    AccelerationStructureBuildPreference build_preference = AccelerationStructureBuildPreference::PreferFastTrace;
};

struct BLASDesc : public AccelerationStructureDesc
{
    std::vector<AccelerationStructureTriangleGeometry> triangle_geometries;
    std::vector<AccelerationStructureAABBGeometry> aabb_geometries;
};

struct TLASDesc : public AccelerationStructureDesc
{
    std::vector<AccelerationStructureInstanceGeometry> instance_geometries;
};

class AccelerationStructure
{
  public:
    ~AccelerationStructure();

    AccelerationStructure(const AccelerationStructure&) = delete;
    AccelerationStructure& operator=(const AccelerationStructure&) = delete;

    AccelerationStructure(AccelerationStructure&&) noexcept;
    AccelerationStructure& operator=(AccelerationStructure&&) noexcept;

    uint64_t device_address() const;

    virtual AccelerationStructureType type() const = 0;

  protected:
    friend class CommandBuffer;
    friend class ResourceSet;

    AccelerationStructure(const PhysicalDevice& physical_device, const Device& device,
                          const AccelerationStructureDesc& desc);

    void create(const PhysicalDevice& physical_device, const Device& device);
    void destroy();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

class BLAS : public AccelerationStructure
{
  public:
    BLAS(const PhysicalDevice& physical_device, const Device& device, const BLASDesc& desc);

    AccelerationStructureType type() const override
    {
        return AccelerationStructureType::BottomLevel;
    }
};

class TLAS : public AccelerationStructure
{
  public:
    TLAS(const PhysicalDevice& physical_device, const Device& device, const TLASDesc& desc);

    AccelerationStructureType type() const override
    {
        return AccelerationStructureType::TopLevel;
    }

    static uint64_t instance_size();
    static void write_instances_to_buffer(const std::vector<AccelerationStructureInstance>& instances,
                                          Buffer& output_buffer);
};

} // namespace ogfx

#endif // OGFX_ACCELERATION_STRUCTURE_HPP