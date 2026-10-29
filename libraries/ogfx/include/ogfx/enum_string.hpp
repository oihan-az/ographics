#ifndef OGFX_ENUM_STRING_HPP
#define OGFX_ENUM_STRING_HPP

#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_flags.hpp>

#include <string>

namespace ogfx
{

template <typename E> std::string to_string_enum(E value)
{
    static_assert(std::is_enum_v<E>, "E must be an enum type");

    const auto name = magic_enum::enum_name(value);

    if (name.empty())
    {
        return "Unknown";
    }

    return std::string(name);
}

template <typename E> std::string to_string_bitmask(E value)
{
    static_assert(std::is_enum_v<E>, "E must be an enum type");

    auto name = magic_enum::enum_flags_name<E>(value);
    if (name.empty())
    {
        using Underlying = std::underlying_type_t<E>;
        return "Unknown(" + std::to_string(static_cast<Underlying>(value)) + ")";
    }

    return std::string(name);
}

}

#endif // OGFX_ENUM_STRING_HPP