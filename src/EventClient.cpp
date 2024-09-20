#include "EventClient.hpp"

namespace ecs::event {

void EventClient::_notify(Event const &e) {
  for (auto const &sub : subscriptions_) {
    sub(e);
  }
}

} // namespace ecs::event
