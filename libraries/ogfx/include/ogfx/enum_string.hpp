#ifndef OGFX_ENUM_STRING_HPP
#define OGFX_ENUM_STRING_HPP

#include <magic_enum/magic_enum.hpp>

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

    using Underlying = std::underlying_type_t<E>;

    const Underlying bits = static_cast<Underlying>(value);

    if (bits == 0)
    {
        return to_string_enum(value);
    }

    using Unsigned = std::make_unsigned_t<Underlying>;

    Unsigned remaining = static_cast<Unsigned>(bits);
    std::string result;

    for (E flag : magic_enum::enum_values<E>())
    {
        const Unsigned flag_bits = static_cast<Unsigned>(static_cast<Underlying>(flag));

        // Only process nonzero, single-bit flags.
        if (flag_bits == 0 || (flag_bits & (flag_bits - 1)) != 0)
        {
            continue;
        }

        if ((remaining & flag_bits) == flag_bits)
        {
            if (!result.empty())
            {
                result += " | ";
            }

            result += to_string_enum(flag);
            remaining &= ~flag_bits;
        }
    }

    if (remaining != 0)
    {
        if (!result.empty())
        {
            result += " | ";
        }

        result += "Unknown(";
        result += std::to_string(static_cast<std::uint64_t>(remaining));
        result += ")";
    }

    return result.empty() ? "Unknown" : result;
}

}

#endif // OGFX_ENUM_STRING_HPP