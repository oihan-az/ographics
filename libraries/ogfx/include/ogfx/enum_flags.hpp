#ifndef OGFX_ENUM_FLAGS_HPP
#define OGFX_ENUM_FLAGS_HPP

#include <type_traits>

#define OGFX_BITMASK_ENUM(EnumName, UnderlyingType, ...)                                                               \
    enum class EnumName : UnderlyingType                                                                               \
    {                                                                                                                  \
        __VA_ARGS__                                                                                                    \
    };                                                                                                                 \
                                                                                                                       \
    constexpr EnumName operator|(EnumName lhs, EnumName rhs)                                                           \
    {                                                                                                                  \
        using Underlying = std::underlying_type_t<EnumName>;                                                           \
        return static_cast<EnumName>(static_cast<Underlying>(lhs) | static_cast<Underlying>(rhs));                     \
    }                                                                                                                  \
                                                                                                                       \
    constexpr EnumName operator&(EnumName lhs, EnumName rhs)                                                           \
    {                                                                                                                  \
        using Underlying = std::underlying_type_t<EnumName>;                                                           \
        return static_cast<EnumName>(static_cast<Underlying>(lhs) & static_cast<Underlying>(rhs));                     \
    }                                                                                                                  \
                                                                                                                       \
    constexpr EnumName operator~(EnumName value)                                                                       \
    {                                                                                                                  \
        using Underlying = std::underlying_type_t<EnumName>;                                                           \
        return static_cast<EnumName>(~static_cast<Underlying>(value));                                                 \
    }                                                                                                                  \
                                                                                                                       \
    constexpr EnumName& operator|=(EnumName& lhs, EnumName rhs)                                                        \
    {                                                                                                                  \
        return lhs = lhs | rhs;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    constexpr EnumName& operator&=(EnumName& lhs, EnumName rhs)                                                        \
    {                                                                                                                  \
        return lhs = lhs & rhs;                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    constexpr bool has_flag(EnumName value, EnumName flag)                                                             \
    {                                                                                                                  \
        using Underlying = std::underlying_type_t<EnumName>;                                                           \
        const auto value_bits = static_cast<Underlying>(value);                                                        \
        const auto flag_bits = static_cast<Underlying>(flag);                                                          \
        return (value_bits & flag_bits) == flag_bits;                                                                  \
    }                                                                                                                  \
                                                                                                                       

#endif // OGFX_ENUM_FLAGS_HPP