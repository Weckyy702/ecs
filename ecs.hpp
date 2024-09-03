#pragma once

#include "Component.hpp"
#include "ComponentStorage.hpp"
#include "EntityID.hpp"
#include "Executor.hpp"
#include "System.hpp"
#include "Type.hpp"
#include <queue>

namespace ECS {

template <Component... Cs> class Ecs {
  using TypeFor = detail::TypeFor<Cs...>;
  using Type = detail::Type<Cs...>;

  constexpr static auto valid_type_bit = sizeof...(Cs);

public:
  template <Component... Ts>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr EntityID create(Ts &&...ts) noexcept {
    constexpr auto type = TypeFor::template getType<Ts...>();

    size_t id = next_id();
    (components_.template update_length<Cs>(types_.size() + 1), ...);

    (components_.insert(id, std::forward<Ts>(ts)), ...);

    if (id < types_.size())
      types_[id] = type;
    else
      types_.push_back(type);

    return EntityID{id};
  }

  template <Component... Ts>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr void add_components(EntityID id, Ts &&...ts) noexcept {
    constexpr auto type = TypeFor::template getType<Ts...>();

    assert(is_valid(id));
    const auto i = id.value_;

    (components_.insert(i, std::forward<Ts>(ts)), ...);
    types_[i] |= type;
  }

  template <Component... Ts>
    requires(contains_v<Ts, Cs...> && ...)
  constexpr void remove_components(EntityID id) noexcept {
    constexpr auto type = TypeFor::template getType<Ts...>();

    assert(is_valid(id));
    const auto i = id.value_;

    (components_.template remove<Ts>(i), ...);
    types_[i] &= ~type;
    types_[i].set(valid_type_bit);
  }
  constexpr void remove(EntityID id) noexcept {
    assert(is_valid(id));

    const auto i = id.value_;
    (components_.template remove<Cs>(i), ...);

    types_[i] = 0u;

    free_ids_.push(i);
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

  constexpr bool is_valid(EntityID id) {
    const auto i = id.value_;
    if (i >= types_.size())
      return false;
    if (!types_[i].test(valid_type_bit))
      return false;
    return true;
  }

  constexpr size_t size() { return types_.size() - free_ids_.size(); }

private:
  template <Component... Ts, System<Ts...> S, Executor E>
    requires(contains_v<std::remove_cv_t<Ts>, Cs...> && ...)
  constexpr void run_impl(const S &s, E e) {
    constexpr auto type = TypeFor::template getType<std::remove_cv_t<Ts>...>();

    e.run(types_.size(), [&](size_t i) {
      if ((types_[i] & type) == type)
        s(components_.template get<std::remove_cv_t<Ts>>(i)...);
    });
  }

  constexpr size_t next_id() noexcept {
    if (free_ids_.empty())
      return types_.size();
    size_t id = free_ids_.front();
    free_ids_.pop();
    return id;
  }

  ComponentStorage<Cs...> components_;
  std::vector<Type> types_;
  std::queue<size_t> free_ids_;
};

} // namespace ECS
