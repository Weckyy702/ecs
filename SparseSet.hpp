#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
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
    if (new_size > sparse.size())
      sparse.resize(new_size, empty_cell);
  }

  constexpr void add(size_t i, C &&c) {
    assert(i < sparse.size());

    if (sparse[i] < dense.size()) {
      Entry e{sparse[i], std::forward<C>(c)};
      (void)e;
      // std::swap(dense[sparse[i]], e);
    } else {
      sparse[i] = dense.size();
      dense.emplace_back(dense.size(), std::forward<C>(c));
    }
  }

  constexpr void remove(size_t i) {
    assert(i < sparse.size());
    if (sparse[i] >= dense.size())
      return;

    std::swap(dense[sparse[i]], dense.back());
    dense.pop_back();

    const auto owner = dense[sparse[i]].backlink;
    assert(sparse[owner] == dense.size());

    sparse[owner] = sparse[i];
    sparse[i] = empty_cell;
  }

  template <typename Self> constexpr auto &get(this Self &self, size_t i) {
    assert(i < self.sparse.size());
    assert(self.sparse[i] < self.dense.size());

    return self.dense[self.sparse[i]].data;
  }

private:
  std::vector<size_t> sparse;
  std::vector<Entry> dense;
};
