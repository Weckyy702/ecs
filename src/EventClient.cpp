#include "ecs/EventClient.hpp"

namespace ECS::Event {

void EventClient::_notify(Event const &e) {
  for (auto const &sub : subscriptions_) {
    sub(e);
  }
}

} // namespace ECS::Event
