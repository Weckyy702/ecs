#pragma once

#include <concepts>
#include <cstddef>

#include <thread>
#include <vector>

namespace ECS {
template <typename E>
concept Executor = requires(E &e) { e.run(size_t{}, [](size_t) {}); };

struct SerialExecutor {
  constexpr void run(size_t num_entities, std::invocable<size_t> auto f) {
    for (auto i = 0uz; i != num_entities; ++i) {
      f(i);
    }
  }
};

class ParallelExecutor {
public:
  ParallelExecutor() {
    if (n_threads == 0)
      n_threads = 1;
  }

  constexpr void run(size_t num_entities, std::invocable<size_t> auto f) {
    std::vector<std::jthread> thread_pool;
    thread_pool.resize(n_threads);

    // Round up
    auto const entities_per_thread = (num_entities - 1) / n_threads + 1;

    for (auto i = 0uz; i < num_entities; i += entities_per_thread) {
      thread_pool.emplace_back([=, &f]() mutable {
        for (auto _ = 0uz; _ != entities_per_thread; ++_) {
          f(i++);
        }
      });
    }
  }

private:
  size_t n_threads{std::jthread::hardware_concurrency()};
};
} // namespace ECS
