#include "ecs/EventManager.hpp"
#include "ecs/EventClient.hpp"

namespace ECS::Event {
std::shared_ptr<EventClient> EventManager::make_client() noexcept {
  auto ptr =
      std::make_shared<EventClient>(shared_from_this(), EventClient::Badge{});

  clients_.emplace_back(ptr);

  return ptr;
}

void EventManager::notify_clients() noexcept {
  while (!events_.empty()) {
    Event const &event = events_.front();
    for (auto it = clients_.begin(); it != clients_.end();) {
      if (auto const client = it->lock(); client) {
        client->_notify(event);
        ++it;
      } else {
        // Remove dead client
        it = clients_.erase(it);
      }
    }
    events_.pop();
  }
}

} // namespace ECS::Event
