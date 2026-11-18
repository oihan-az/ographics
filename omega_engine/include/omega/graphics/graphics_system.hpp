#ifndef OMEGA_GRAPHICS_SYSTEM_HPP
#define OMEGA_GRAPHICS_SYSTEM_HPP

#include <orenderer/renderer.hpp>

#include <omega/mesh/mesh_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/texture/texture_system.hpp>
#include <omega/texture/gpu_texture_system.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class PhysicalDevice;
class Device;
class ResourceAllocator;
class Swapchain;
class Fence;
class Semaphore;

} // namespace ogfx

namespace omega
{

class GraphicsSystem
{
  public:
    GraphicsSystem(ogfx::PhysicalDevice& physical_device, ogfx::Device& device,
                   ogfx::ResourceAllocator& allocator, ogfx::Swapchain& swapchain, 
                   uint32_t max_frames_in_flight, EventSystem& events);
    ~GraphicsSystem();

    void start_frame();
    void end_frame();

    void initialize();
    void update();
    void render();

    uint32_t current_frame() const;
    uint32_t current_image_index() const;
    uint32_t max_frames_in_flight() const;

    ogfx::Swapchain& swapchain();

    orenderer::Renderer& renderer();

    MeshSystem& meshes();
    GpuMeshSystem& gpu_meshes();

    TextureSystem& textures();
    GpuTextureSystem& gpu_textures();

    TextureHandle missing_texture() const;
    TextureViewHandle missing_texture_view() const;

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

    std::vector<Frame> m_frames;
    std::vector<std::unique_ptr<ogfx::Semaphore>> m_render_finished;

    const uint32_t m_max_frames_in_flight = 0;
    uint32_t m_current_frame = 0;
    uint32_t m_current_image_index = 0;

    ogfx::Device& m_device;
    ogfx::Swapchain& m_swapchain;

    orenderer::Renderer m_renderer;

    MeshSystem m_meshes;
    GpuMeshSystem m_gpu_meshes;

    TextureSystem m_textures;
    GpuTextureSystem m_gpu_textures;

    TextureHandle m_missing_texture;
    TextureViewHandle m_missing_texture_view;

    void wait();
    void complete_uploads();

    static TextureAsset create_missing_texture_asset();
};

} // namespace omega

#endif // OMEGA_GRAPHICS_SYSTEM_HPP