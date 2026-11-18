#ifndef OGFX_INSTANCE_EXTENSIONS_HPP
#define OGFX_INSTANCE_EXTENSIONS_HPP

#include <ogfx/enum_flags.hpp>

#include <cstdint>

namespace ogfx
{

OGFX_BITMASK_ENUM(InstanceExtension, uint64_t,
    None = 0,

    // Surface
    Surface = 1ull << 0,
    Win32Surface = 1ull << 1,
    XlibSurface = 1ull << 2,
    XcbSurface = 1ull << 3,
    WaylandSurface = 1ull << 4,
    AndroidSurface = 1ull << 5,
    MetalSurface = 1ull << 6,
    MacOSSurface = 1ull << 7,
    HeadlessSurface = 1ull << 8,

    // Debugging and validation
    DebugUtils = 1ull << 9,
    DebugReport = 1ull << 10,
    ValidationFeatures = 1ull << 11,
    ValidationFlags = 1ull << 12,
    LayerSettings = 1ull << 13,

    // Physical device and device discovery
    GetPhysicalDeviceProperties2 = 1ull << 14,
    DeviceGroupCreation = 1ull << 15,
    ToolingInfo = 1ull << 16,

    // External resource capabilities
    ExternalMemoryCapabilities = 1ull << 17,
    ExternalSemaphoreCapabilities = 1ull << 18,
    ExternalFenceCapabilities = 1ull << 19,

    // Surface capabilities
    GetSurfaceCapabilities2 = 1ull << 20,
    SurfaceProtectedCapabilities = 1ull << 21,
    SurfaceMaintenance1 = 1ull << 22,
    SwapchainColorspace = 1ull << 23,

    // Display
    Display = 1ull << 24,
    GetDisplayProperties2 = 1ull << 25,
    DirectModeDisplay = 1ull << 26,
    AcquireDrmDisplay = 1ull << 27,

    // Portability
    PortabilityEnumeration = 1ull << 28
);

} // namespace ogfx

#endif // OGFX_INSTANCE_EXTENSIONS_HPP