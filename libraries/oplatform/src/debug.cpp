#include "debug.hpp"

#include <filesystem>
#include <iostream>

namespace oplatform
{

void log(LogLevel level, std::string_view message, std::string_view file, int line)
{
    std::string_view level_name;

    switch (level)
    {
    case LogLevel::Debug:
        level_name = "DEBUG";
        break;

    case LogLevel::Info:
        level_name = "INFO";
        break;

    case LogLevel::Warning:
        level_name = "WARNING";
        break;

    case LogLevel::Error:
        level_name = "ERROR";
        break;
    }

    const std::filesystem::path path(file);

    std::cout 
        << "[oplatform][" << level_name << "] " 
        << path.lexically_relative(OPLATFORM_SOURCE_DIR).string() 
        << ':' 
        << line        
        << " - " << message << '\n';
}

} // namespace oplatform