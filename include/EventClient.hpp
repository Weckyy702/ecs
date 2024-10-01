#pragma once

#include <functional>
#include <memory>

#include "EventManager.hpp"

namespace ECS::Event {

class EventClient {
  friend class EventManager;
  // Only our friend EventManager can call our constructor now
  struct Badge {};

  using Subscription = std::move_only_function<void(Event const &) const>;

public:
  EventClient(std::shared_ptr<EventManager> manager, Badge)
      : manager_{manager} {}

  template <typename T, std::invocable<T const &> F> void subscribe(F &&f) {
    subscriptions_.emplace_back([f](Event const &e) {
      if (e.is<T>()) {
        f(e.as<T>());
      }
    });
  }

  template <typename E> void emit(E &&e) noexcept {
    manager_->emit(std::forward<E>(e));
  }

private:
  void _notify(Event const &);

  std::shared_ptr<EventManager> manager_;
  std::vector<Subscription> subscriptions_;
};

} // namespace ECS::Event
