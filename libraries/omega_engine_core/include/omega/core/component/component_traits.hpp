#ifndef OMEGA_COMPONENT_TRAITS_HPP
#define OMEGA_COMPONENT_TRAITS_HPP

namespace omega
{

template<typename... T>
struct TypeList {};

template<typename T>
struct ComponentTraits
{
    using Dependencies = TypeList<>;
};

} // namespace omega

#endif // OMEGA_COMPONENT_TRAITS_HPP