#pragma once

#include <memory>
#include <queue>
#include <vector>

#include "Event.hpp"

namespace ecs::event {
class EventClient;

class EventManager : public std::enable_shared_from_this<EventManager> {
  struct Badge {};

  friend class EventClient;

public:
  EventManager(Badge) {}

  static std::shared_ptr<EventManager> make() noexcept {
    return std::make_shared<EventManager>(Badge{});
  }

  bool has_pending_events() const noexcept { return !events_.empty(); }

  std::shared_ptr<EventClient> make_client() noexcept;

  template <typename T> void emit(T &&t) noexcept {
    _emit(Event{std::forward<T>(t)});
  }

  void notify_clients() noexcept;

private:
  void _emit(Event) noexcept;
  std::vector<std::weak_ptr<EventClient>> clients_;
  std::queue<Event> events_;
};
} // namespace ecs::event
