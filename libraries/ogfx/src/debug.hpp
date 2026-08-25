#ifndef OGFX_DEBUG_HPP
#define OGFX_DEBUG_HPP

#include <string_view>
#include <sstream>

namespace ogfx
{

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

void log(LogLevel level, std::string_view message, std::string_view file, int line);

} // namespace ogfx

#ifndef NDEBUG

#define OGFX_LOG(message) ::ogfx::log(::ogfx::LogLevel::Info, (message), __FILE__, __LINE__)

#define OGFX_LOG_WARNING(message) ::ogfx::log(::ogfx::LogLevel::Warning, (message), __FILE__, __LINE__)

#define OGFX_LOG_ERROR(message) ::ogfx::log(::ogfx::LogLevel::Error, (message), __FILE__, __LINE__)

#else

#define OGFX_LOG(message) ((void)0)

#endif

#endif // OGFX_DEBUG_HPP