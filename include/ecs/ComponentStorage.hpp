#pragma once

#include "Component.hpp"
#include "SparseSet.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace ECS {
template <Component C> class ComponentStorageImpl {
public:
  template <typename U>
    requires(std::is_constructible_v<C, U>)
  constexpr void insert(size_t id, U &&u) {
    entities_.add(id, std::forward<U>(u));
  }

  constexpr void remove(size_t i) { entities_.remove(i); }

  constexpr void update_length(size_t l) { entities_.reserve(l); }

  constexpr C &get(size_t i) { return entities_.get(i); }

private:
  SparseSet<C> entities_;
};

template <Component... Cs>
class ComponentStorage : ComponentStorageImpl<Cs>... {
public:
  using ComponentStorageImpl<Cs>::insert...;

  template <Component C>
    requires contains_v<C, Cs...>
  constexpr void remove(size_t i) {
    ComponentStorageImpl<C>::remove(i);
  }

  template <Component C>
    requires contains_v<C, Cs...>
  constexpr C &get(size_t i) {
    return ComponentStorageImpl<C>::get(i);
  }

  template <Component C>
    requires contains_v<C, Cs...>
  constexpr void update_length(size_t i) {
    ComponentStorageImpl<C>::update_length(i);
  }
};
} // namespace ECS
