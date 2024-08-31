#include <chrono>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string_view>
#include <thread>

#include "Executor.hpp"
#include "System.hpp"
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

struct GravitySystem : public ECS::BaseSystem<GravitySystem, Physics, Gravity> {
  void run(Physics &p, Gravity const &) const { p.acceleration.y -= 9.81; }
};

struct Printer : public ECS::BaseSystem<Printer, Index, Position> {
  void run(Index const &i, Position const &p) const {
    const auto [x, y] = p.position;
    fmt::println("Entity {}: ({}, {})", i.i, x, y);
  }
};

int main() {
  using ECS = ECS::Ecs<Index, Position, Physics, Gravity>;

  constexpr auto N = 100'000'000uz;

  ECS ecs{};

  [[maybe_unused]] const auto time = [](std::invocable auto f,
                                        std::string_view label) {
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

    time([&] { ecs.run(GravitySystem{}); }, "Gravity update");

    time(
        [&] {
          ecs.run(
              +[](Position &pos, Physics &phy) {
                phy.velocity += phy.acceleration;
                pos.position += phy.velocity;
                phy.acceleration = Vec2{};
              },
              ::ECS::ParallelExecutor{});
        },
        "Physics update");

    using std::chrono_literals::operator""ms;
    std::this_thread::sleep_for(500ms);
  }
}
