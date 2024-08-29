#pragma once

#include "Component.hpp"

#include <compare>
#include <cstddef>

namespace ECS {

class EntityID {
public:
  template <Component...> friend class Ecs;

  constexpr auto operator<=>(EntityID const &) const = default;

private:
  constexpr explicit EntityID(size_t value) : value_{value} {}

  size_t value_;
};

} // namespace ECS
