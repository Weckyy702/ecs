#include <chrono>
#include <fmt/core.h>
#include <fmt/format.h>
#include <random>
#include <string_view>
#include <thread>

#include "ecs/EntityID.hpp"
#include "ecs/System.hpp"
#include "ecs/ecs.hpp"

namespace {

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
  static constexpr double acceleration = 9.81;

  static void run(Physics &phys, Gravity const & /*unused*/) {
    phys.acceleration.y -= acceleration;
  }
};

struct Printer : public ECS::BaseSystem<Printer, Index, Position> {
  static void run(Index const &idx, Position const &phys) {
    const auto [x, y] = phys.position;
    fmt::println("Entity {}: ({}, {})", idx.i, x, y);
  }
};

[[maybe_unused]] void perf_test(size_t runs = 250'000'000UZ) {
  using ECS = ECS::Ecs<Index, Position, Physics, Gravity>;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;

  ECS ecs{};

  [[maybe_unused]] constexpr auto time = [](std::invocable auto f,
                                            std::string_view label) {
    const auto start = high_resolution_clock::now();
    f();
    const auto end = high_resolution_clock::now();

    const auto elapsed = duration_cast<milliseconds>(end - start);
    fmt::println("{}: {}ms", label, elapsed.count());
    return elapsed;
  };

  ecs.reserve(runs);

  std::mt19937 rng{std::random_device{}()};
  std::bernoulli_distribution dist{0.25};

  for (auto i = 0UZ; i != runs; ++i) {
    const auto id = ecs.create(Index{i});

    if (dist(rng)) {
      ecs.add_components(id, Position{});
    }

    if (dist(rng)) {
      ecs.add_components(id, Physics{}, Gravity{});
    }
  }

  while (true) {

    time([&] { ecs.run(GravitySystem{}); }, "Gravity update");

    time(
        [&] {
          ecs.run(+[](Position &pos, Physics &phy) {
            phy.velocity += phy.acceleration;
            pos.position += phy.velocity;
            phy.acceleration = Vec2{};
          });
        },
        "Physics update");

    using std::chrono_literals::operator""ms;
    std::this_thread::sleep_for(500ms);
  }
}

struct Counter : ECS::BaseSystem<Counter> {
  static size_t counter;

  static void run() noexcept { counter++; }
};
size_t Counter::counter = 0;

[[maybe_unused]] void remove_test() {
  using Ecs = ECS::Ecs<Index, Position>;

  Ecs ecs{};

  std::vector<ECS::EntityID> ids;

  for (auto i = 0UZ; i != 10; ++i) {
    const auto id =
        ecs.create(Index{i}, Position{{.x = static_cast<double>(i), .y = 0}});
    ids.push_back(id);
  }

  ecs.run(Printer{});

  fmt::println("------");

  for (auto i = 0UZ; i != ids.size() / 2; ++i) {
    const auto id = ids[2 * i];
    ecs.remove_components<Position>(id);
  }

  ecs.run(Printer{});

  ecs.remove(ids[1]);

  assert(!ecs.is_valid(ids[1]));
  assert(ecs.create(Index{1}) == ids[1]);

  ecs.run(+[](Index const &idx) { fmt::println("Entity {}", idx.i); });

  ecs.run(Counter{});
  fmt::println("{} entities - {} entities", Counter::counter, ecs.size());
}
} // namespace

int main() { remove_test(); }
