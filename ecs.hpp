#pragma once

#include "Component.hpp"
#include "ComponentStorage.hpp"
#include "EntityID.hpp"
#include "Executor.hpp"
#include "System.hpp"
#include "Type.hpp"

namespace ECS {

template <Component... Cs> class Ecs {
  using TypeFor = detail::TypeFor<Cs...>;
  using Type = detail::Type<Cs...>;

public:
  template <Component... Ts>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr EntityID create(Ts &&...ts) noexcept {
    size_t id = num_entities++;
    (components_.template update_length<Cs>(num_entities), ...);

    (components_.insert(id, std::forward<Ts>(ts)), ...);

    types_.push_back(TypeFor::template getType<Ts...>());
    return EntityID{id};
  }

  template <Component... Ts>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr void add_components(EntityID id, Ts &&...ts) noexcept {
    constexpr auto type = TypeFor::template getType<Ts...>();

    const auto i = id.value_;

    assert(i < types_.size());

    (components_.insert(i, std::forward<Ts>(ts)), ...);
    types_[i] |= type;
  }

  template <typename Derived, Component... Ts, Executor E = SerialExecutor>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr void run(BaseSystem<Derived, Ts...> const &s, E e = {}) {
    run_impl<Ts...>(s, e);
  }

  template <Component... Ts, Executor E = SerialExecutor>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr void run(void (*fn)(Ts &...), E e = {}) {
    run_impl<Ts...>([=](Ts &...ts) { fn(ts...); }, e);
  }

  void reserve(size_t n) { (components_.template update_length<Cs>(n), ...); }

private:
  template <Component... Ts, System<Ts...> S, Executor E>
    requires(contains_v<std::remove_cv_t<Ts>, Cs...> && ...)
  constexpr void run_impl(const S &s, E e) {
    constexpr auto type = TypeFor::template getType<std::remove_cv_t<Ts>...>();

    e.run(num_entities, [&](size_t i) {
      if ((types_[i] & type) == type)
        s(components_.template get<std::remove_cv_t<Ts>>(i)...);
    });
  }

  }

  ComponentStorage<Cs...> components_;
  std::vector<Type> types_;
  size_t num_entities;
};

} // namespace ECS
