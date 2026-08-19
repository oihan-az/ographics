/**
 * @file window.hpp
 * @brief Platform-independent window abstraction.
 */

#ifndef OPLATFORM_WINDOW_HPP
#define OPLATFORM_WINDOW_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace oplatform
{

/**
 * @brief Describes the properties of a window.
 */
struct WindowDesc
{
    uint32_t width = 1280;
    uint32_t height = 720;

    std::string title = "ographics";

    bool resizable = true;
    bool fullscreen = false;
};

/**
 * @brief Represents an application window.
 *
 * Window provides a platform-independent interface for creating and
 * interacting with application windows.
 *
 * A valid oplatform::Runtime must exist before creating a Window and
 * must outlive all Window instances.
 */
class Window
{
  public:

    /**
     * @brief Creates a window from the given description.
     *
     * @param desc Properties used to configure the window.
     *
     * @throws std::runtime_error if the window cannot be created.
     */
    explicit Window(const WindowDesc& desc);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

    void poll_events();

    [[nodiscard]] bool should_close() const;
    [[nodiscard]] uint32_t width() const;
    [[nodiscard]] uint32_t height() const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace oplatform

#endif // OPLATFORM_WINDOW_HPP