#ifndef OMEGA_GRAPHICS_SYSTEM_HPP
#define OMEGA_GRAPHICS_SYSTEM_HPP

#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/surface.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/swapchain.hpp>

#include <orenderer/renderer.hpp>

#include <omega/mesh/mesh_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/texture/texture_system.hpp>
#include <omega/texture/gpu_texture_system.hpp>

#include <omega/core/performance/performance_system.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class Fence;
class Semaphore;

} // namespace ogfx

namespace omega
{

class PlatformSystem;

class GraphicsSystem
{
  public:
    GraphicsSystem(const std::string& application_name, uint32_t max_frames_in_flight, 
                   PlatformSystem& platform, EventSystem& events, PerformanceSystem& performance);
    ~GraphicsSystem();

    void start_frame();
    void end_frame();

    void initialize();
    void update();
    void render();
    void wait();

    uint32_t current_frame() const;
    uint32_t current_image_index() const;
    uint32_t max_frames_in_flight() const;

    bool needs_swapchain_recreation() const;
    void recreate_swapchain(uint32_t width, uint32_t height);

    ogfx::Swapchain& swapchain();

    orenderer::Renderer& renderer();

    MeshSystem& meshes();
    GpuMeshSystem& gpu_meshes();

    ogfx::Instance& instance();
    ogfx::PhysicalDevice& physical_device();
    ogfx::Device& device();
    ogfx::ResourceAllocator& resource_allocator();

    TextureSystem& textures();
    GpuTextureSystem& gpu_textures();

    TextureHandle missing_texture() const;
    TextureViewHandle missing_texture_view() const;

    ogfx::CommandBuffer& current_command_buffer();

  private:
    struct Frame
    {
        std::unique_ptr<ogfx::CommandPool> command_pool;
        std::unique_ptr<ogfx::CommandBuffer> command_buffer;
        std::unique_ptr<ogfx::CommandBuffer> upload_command_buffer;

        std::unique_ptr<ogfx::Semaphore> image_available;
        std::unique_ptr<ogfx::Fence> in_flight;

        TextureUploadBatch upload_batch{};
    };

    const uint32_t m_max_frames_in_flight = 0;
    uint32_t m_current_frame = 0;
    uint32_t m_current_image_index = 0;

    ogfx::AcquireResult m_frame_acquired;
    bool m_recreate_swapchain = false;

    ogfx::Instance m_instance;
    ogfx::Surface m_surface;
    ogfx::PhysicalDevice m_physical_device;
    ogfx::Device m_device;
    ogfx::Swapchain m_swapchain;
    ogfx::ResourceAllocator m_resource_allocator;

    std::vector<Frame> m_frames;
    std::vector<std::unique_ptr<ogfx::Semaphore>> m_render_finished;

    orenderer::Renderer m_renderer;

    MeshSystem m_meshes;
    GpuMeshSystem m_gpu_meshes;

    TextureSystem m_textures;
    GpuTextureSystem m_gpu_textures;

    TextureHandle m_missing_texture;
    TextureViewHandle m_missing_texture_view;

    void wait_frame();
    void complete_uploads();

    static TextureAsset create_missing_texture_asset();

    PerformanceSystem& m_performance;
};

} // namespace omega

#endif // OMEGA_GRAPHICS_SYSTEM_HPP