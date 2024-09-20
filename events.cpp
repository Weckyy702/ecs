
#include "EventClient.hpp"
#include "EventManager.hpp"
#include <iostream>

struct MyEvent {
  int i;
};

struct YourEvent {
  int i;
};

int main() {
  auto manager = ecs::event::EventManager::make();

  auto sender = manager->make_client();
  auto my_receiver = manager->make_client();
  auto your_receiver = manager->make_client();

  my_receiver->subscribe<MyEvent>(
      [](MyEvent const &e) { std::println(std::cout, "MyEvent {}", e.i); });
  your_receiver->subscribe<YourEvent>(
      [](YourEvent const &e) { std::println(std::cout, "YourEvent {}", e.i); });

  sender->emit(MyEvent{12});
  sender->emit(YourEvent{14});

  manager->notify_clients();
}
