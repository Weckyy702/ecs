#include "ecs/EventClient.hpp"
#include "ecs/EventManager.hpp"
#include <iostream>
#include <string>

struct MyEvent {
  int i;
};

struct YourEvent {
  int i;
  std::string s;
};

int main() {
  auto manager = ECS::Event::EventManager::make();

  auto sender = manager->make_client();
  auto my_receiver = manager->make_client();
  auto your_receiver = manager->make_client();

  my_receiver->subscribe<MyEvent>(
      [](MyEvent const &e) { std::println(std::cout, "MyEvent {}", e.i); });
  your_receiver->subscribe<YourEvent>([](YourEvent const &e) {
    std::println(std::cout, "YourEvent {} {}", e.i, e.s);
  });

  sender->emit(MyEvent{12});
  sender->emit(YourEvent{14, "ayo"});

  manager->notify_clients();
}
