#pragma once

#include "Component.hpp"

#include <bitset>

namespace ECS::detail {
template <Component... Cs> using Type = std::bitset<sizeof...(Cs)>;

template <Component... Cs> struct TypeFor {
  template <Component... Ts> constexpr static Type<Cs...> getType() {
    return {(bit_for<Ts, Cs...> | ... | 0)};
  }
};

} // namespace ECS
