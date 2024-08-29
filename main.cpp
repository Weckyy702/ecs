#include <chrono>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string_view>
#include <thread>

#include "ecs.hpp"

struct Vec2 {
  constexpr Vec2 &operator+=(const Vec2 &other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  double x, y;
};

struct Position {
  Vec2 position;
};

struct Physics {
  Vec2 velocity;
  Vec2 acceleration;
};

struct Gravity {};

struct Index {
  size_t i;
};

int main() {
  using ECS = ECS::Ecs<Index, Position, Physics, Gravity>;

  constexpr auto N = 250'000'000uz;

  ECS ecs{};

  const auto time = [](std::invocable auto f, std::string_view label) {
    using namespace std::chrono;
    const auto start = high_resolution_clock::now();
    f();
    const auto end = high_resolution_clock::now();

    const auto elapsed = duration_cast<milliseconds>(end - start);
    fmt::println("{}: {}ms", label, elapsed.count());
    return elapsed;
  };

  ecs.reserve(N);

  for (auto i = 0uz; i != N; ++i) {
    const auto id = ecs.create(Index{i});

    if (i % 3 == 0) {
      ecs.add_components(id, Position{});
    }

    if (i % 6 == 0) {
      ecs.add_components(id, Physics{}, Gravity{});
    }
  }

  while (true) {
    time(
        [&] {
          ecs.run(
              +[](Physics &p, Gravity const &) { p.acceleration.y -= 9.81; });
        },
        "Gravity update");

    time(
        [&] {
          ecs.run(+[](Position &pos, Physics &phy) {
            phy.velocity += phy.acceleration;
            pos.position += phy.velocity;
            phy.acceleration = Vec2{};
          });
        },
        "Physics update");

    /*ecs.run(+[](const Position &pos, const Index &idx) {
      if (idx.i > 10)
        return;
      const auto [x, y] = pos.position;
      fmt::println("Entity {}: ({}, {})", idx.i, x, y);
    });*/

    using std::chrono_literals::operator""ms;
    std::this_thread::sleep_for(500ms);
  }
}
