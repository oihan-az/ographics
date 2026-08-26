#ifndef OPLATFORM_DEBUG_HPP
#define OPLATFORM_DEBUG_HPP

#include <string_view>
#include <sstream>

namespace oplatform
{

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

void log(LogLevel level, std::string_view message, std::string_view file, int line);

} // namespace oplatform

#ifndef NDEBUG

#define OPLATFORM_LOG(message) ::oplatform::log(::oplatform::LogLevel::Info, (message), __FILE__, __LINE__)

#define OPLATFORM_LOG_WARNING(message) ::oplatform::log(::oplatform::LogLevel::Warning, (message), __FILE__, __LINE__)

#define OPLATFORM_LOG_ERROR(message) ::oplatform::log(::oplatform::LogLevel::Error, (message), __FILE__, __LINE__)

#else

#define OPLATFORM_LOG(message) ((void)0)

#endif

#endif // OPLATFORM_DEBUG_HPP