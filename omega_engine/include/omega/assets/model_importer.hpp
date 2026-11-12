#ifndef OMEGA_MODEL_IMPORTER_HPP
#define OMEGA_MODEL_IMPORTER_HPP

#include <omega/assets/imported_model.hpp>

#include <filesystem>

namespace omega
{

class ModelImporter
{
  public:
    virtual ~ModelImporter() = default;

    virtual ImportedModel import(const std::filesystem::path& path);
};

} // namespace omega

#endif // OMEGA_MODEL_IMPORTER_HPP