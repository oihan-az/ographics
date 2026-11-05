#ifndef LOADER_HPP
#define LOADER_HPP

#include <fstream>
#include <filesystem>
#include <cstdint>
#include <vector>

std::vector<uint8_t> load_shader(const std::filesystem::path& path);

#endif // LOADER_HPP