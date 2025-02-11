#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <typename C> class SparseSet {

  struct Entry {
    size_t backlink;
    C data;
  };

public:
  constexpr static auto empty_cell = std::numeric_limits<size_t>::max();

  constexpr void reserve(size_t new_size) {
    if (new_size > sparse.size()) {
      sparse.resize(new_size, empty_cell);
    }
  }

  template <typename U>
    requires std::is_constructible_v<C, U>
  constexpr void add(size_t idx, U &&arg) {
    assert(idx < sparse.size());

    if (sparse[idx] < dense.size()) {
      dense[sparse[idx]] = Entry{sparse[idx], std::forward<U>(arg)};
    } else {
      sparse[idx] = dense.size();
      dense.emplace_back(dense.size(), std::forward<U>(arg));
    }
  }

  constexpr void remove(size_t idx) {
    assert(idx < sparse.size());
    if (sparse[idx] >= dense.size()) {
      return;
    }

    std::swap(dense[sparse[idx]], dense.back());
    dense.pop_back();

    const auto owner = dense[sparse[idx]].backlink;
    assert(sparse[owner] == dense.size());

    sparse[owner] = sparse[idx];
    sparse[idx] = empty_cell;
  }

  template <typename Self> constexpr auto &get(this Self &self, size_t idx) {
    assert(idx < self.sparse.size());
    assert(self.sparse[idx] < self.dense.size());

    return self.dense[self.sparse[idx]].data;
  }

private:
  std::vector<size_t> sparse;
  std::vector<Entry> dense;
};
