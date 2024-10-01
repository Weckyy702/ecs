#include "EventManager.hpp"
#include "EventClient.hpp"

namespace ECS::Event {
std::shared_ptr<EventClient> EventManager::make_client() noexcept {
  auto ptr =
      std::make_shared<EventClient>(shared_from_this(), EventClient::Badge{});

  clients_.emplace_back(ptr);

  return ptr;
}

void EventManager::emit(Event e) noexcept { events_.emplace(std::move(e)); }

static void
remove_dead_clients(std::vector<std::weak_ptr<EventClient>> &clients) {
  for (auto it = clients.begin(); it != clients.end();) {
    auto const client = it->lock();
    if (!client) {
      it = clients.erase(it);
      continue;
    }
    it++;
  }
}

void EventManager::notify_clients() noexcept {
  remove_dead_clients(clients_);

  while (!events_.empty()) {
    Event const &e = events_.front();
    for (auto const &client : clients_) {
      client.lock()->_notify(e);
    }
    events_.pop();
  }
}

} // namespace ECS::Event
