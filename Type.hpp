#pragma once

#include "Component.hpp"

#include <bitset>

namespace ECS::detail {
template <Component... Cs> using Type = std::bitset<sizeof...(Cs) + 1>;

template <Component... Cs> struct TypeFor {
  template <Component... Ts> constexpr static Type<Cs...> getType() {
    constexpr auto valid_bit = 1u << sizeof...(Cs);
    return {(bit_for<Ts, Cs...> | ... | valid_bit)};
  }
};

} // namespace ECS::detail
