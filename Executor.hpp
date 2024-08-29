#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>

#include <thread>
#include <vector>

namespace ECS {
template <typename E>
concept Executor = requires(E &e) { e.run(size_t{}, [](size_t) {}); };

struct SerialExecutor {
  template <std::invocable<size_t> F>
  constexpr void run(size_t num_entities, F f) {
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

  template <std::invocable<size_t> F>
  constexpr void run(size_t num_entities, F f) {
    constexpr auto order = std::memory_order_relaxed;

    std::vector<std::jthread> thread_pool;
    thread_pool.reserve(n_threads);

    std::atomic_size_t index{};
    for (auto _ = 0uz; _ != thread_pool.capacity(); ++_) {
      thread_pool.emplace_back([&] {
        auto i = index.fetch_add(1, order);
        while (i < num_entities) {
          f(i);
          i = index.fetch_add(1, order);
        }
      });
    }
  }

private:
  size_t n_threads{std::jthread::hardware_concurrency()};
};
} // namespace ECS
