#pragma once

#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "Event.hpp"

namespace ECS::Event {
class EventClient;

class EventManager : public std::enable_shared_from_this<EventManager> {
  struct Badge {};

  friend class EventClient;

public:
  explicit EventManager(Badge) {}

  static std::shared_ptr<EventManager> make() noexcept {
    return std::make_shared<EventManager>(Badge{});
  }

  bool has_pending_events() const noexcept { return !events_.empty(); }

  std::shared_ptr<EventClient> make_client() noexcept;

  template <typename T> void emit(T &&t) noexcept {
    events_.emplace(std::forward<T>(t));
  }

  void notify_clients() noexcept;

private:
  std::vector<std::weak_ptr<EventClient>> clients_;
  std::queue<Event> events_;
};
} // namespace ECS::Event
