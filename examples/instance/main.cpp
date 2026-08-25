#include <ogfx/instance.hpp>
#include <ogfx/device.hpp>

int main()
{
    ogfx::InstanceDesc instanceDesc;
    instanceDesc.application_name = "ogfx Instance Test";
    instanceDesc.engine_name = "ographics";
    instanceDesc.debug = true;

    ogfx::Instance instance(instanceDesc);

    const std::vector<ogfx::PhysicalDevice> devices = instance.enumerate_physical_devices();
    ogfx::PhysicalDevice physicalDevice = instance.pick_physical_device(devices);

    physicalDevice.properties();

    ogfx::DeviceDesc deviceDesc;
    deviceDesc.enable_transfer = true;
    deviceDesc.enable_graphics = true;
    deviceDesc.enable_compute = true;

    ogfx::Device device(physicalDevice, deviceDesc);

    return 0;
}