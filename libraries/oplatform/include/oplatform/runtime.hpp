/**
 * @file runtime.hpp
 * @brief Platform runtime management.
 */

#ifndef OPLATFORM_RUNTIME_HPP
#define OPLATFORM_RUNTIME_HPP

#include <vector>

namespace oplatform
{

/**
 * @brief Owns the lifetime of the oplatform runtime.
 *
 * Runtime is responsible for initializing the underlying platform backend
 * when constructed and shutting it down when destroyed.
 *
 * An application should create exactly one Runtime instance. The Runtime
 * must outlive all oplatform resources created by the application.
 *
 * Runtime is neither copyable nor movable.
 */
class Runtime
{
  public:

    /**
     * @brief Initializes the platform runtime.
     *
     * @throws std::runtime_error if initialization fails.
     */
    Runtime();

    /**
     * @brief Shuts down the platform runtime.
     */
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    [[nodiscard]] std::vector<const char*> required_instance_extensions() const;
};
} // namespace oplatform

#endif // OPLATFORM_RUNTIME_HPP