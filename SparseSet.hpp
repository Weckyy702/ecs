#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

template <typename C> class SparseSet {
public:
  constexpr static auto empty_cell = std::numeric_limits<size_t>::max();

  constexpr void reserve(size_t new_size) {
    if (new_size > sparse.size())
      sparse.resize(new_size, empty_cell);
  }

  constexpr void add(size_t i, C &&c) {
    assert(i < sparse.size());
    if (sparse[i] < dense.size()) {
      dense[sparse[i]] = std::forward<C>(c);
      return;
    }

    sparse[i] = dense.size();
    dense.push_back(std::forward<C>(c));
  }

  template <typename Self> constexpr auto &get(this Self &self, size_t i) {
    assert(i < self.sparse.size());
    assert(self.sparse[i] < self.dense.size());

    return self.dense[self.sparse[i]];
  }

private:
  std::vector<size_t> sparse;
  std::vector<C> dense;
};
