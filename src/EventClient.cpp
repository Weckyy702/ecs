#include "ecs/EventClient.hpp"

namespace ECS::Event {

void EventClient::_notify(Event const &event) const {
  for (auto const &sub : subscriptions_) {
    sub(event);
  }
}

} // namespace ECS::Event
