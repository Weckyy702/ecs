#pragma once

#include "Component.hpp"

namespace ECS {
template <typename T, typename... Cs>
concept System = requires(T const &s, Cs &...cs) { s(cs...); };

template <typename Derived, Component... Cs> struct BaseSystem {

  void operator()(Cs &...cs) const
    requires requires(Derived const &s, Cs &...cs) { s.run(cs...); }
  {
    static_cast<Derived const *>(this)->run(cs...);
  }
};

} // namespace ECS
