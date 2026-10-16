#ifndef OGFX_ENUM_FLAGS_HPP
#define OGFX_ENUM_FLAGS_HPP

#define OGFX_BITMASK_ENUM(EnumName, UnderlyingType, ...)                                                               \
    enum class EnumName : UnderlyingType                                                                               \
    {                                                                                                                  \
        __VA_ARGS__                                                                                                    \
    };                                                                                                                 \
                                                                                                                       \
    constexpr EnumName operator|(EnumName lhs, EnumName rhs)                                                           \
    {                                                                                                                  \                                                         \
        return static_cast<EnumName>(static_cast<UnderlyingType>(lhs) | static_cast<UnderlyingType>(rhs));             \
    }                                                                                                                  \
                                                                                                                       \
    constexpr EnumName operator&(EnumName lhs, EnumName rhs)                                                           \
    {                                                                                                                  \                                                         \
        return static_cast<EnumName>(static_cast<UnderlyingType>(lhs) & static_cast<UnderlyingType>(rhs));             \
    }                                                                                                                  \
                                                                                                                       \
    constexpr EnumName operator~(EnumName value)                                                                       \
    {                                                                                                                  \                                                          \
        return static_cast<EnumName>(~static_cast<UnderlyingType>(value));                                             \
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
    {                                                                                                                  \                                                         \
        const auto value_bits = static_cast<UnderlyingType>(value);                                                    \
        const auto flag_bits = static_cast<UnderlyingType>(flag);                                                      \
        return (value_bits & flag_bits) == flag_bits;                                                                  \
    }

#endif // OGFX_ENUM_FLAGS_HPP