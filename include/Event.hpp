#pragma once

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace ECS::Event {

namespace details {
union EventStorage {
  void *heap_allocated_{};
  alignas(std::max_align_t) std::byte
      inline_storage_[alignof(std::max_align_t)];
};

struct EventVTable {
  void (*move)(EventStorage &dst, EventStorage &src);
  void const *(*get)(EventStorage const &);
  void (*destory)(EventStorage &);
};

template <typename, bool> struct MakeEventVTable;

// Object is stored inline
template <typename T> struct MakeEventVTable<T, true> {
  static constexpr void construct(EventStorage &storage, T obj) {
    new (storage.inline_storage_) T{std::move(obj)};
  }

  static constexpr void move(EventStorage &dst, EventStorage &src) {
    new (dst.inline_storage_)
        T{std::move(*reinterpret_cast<T *>(src.inline_storage_))};
  }

  static constexpr void const *get(EventStorage const &storage) {
    return storage.inline_storage_;
  }

  static constexpr void destroy(EventStorage &storage) {
    reinterpret_cast<T *>(storage.inline_storage_)->~T();
  }
};

template <typename T> struct MakeEventVTable<T, false> {
  static constexpr void construct(EventStorage &storage, T obj) {
    storage.heap_allocated_ = new T{std::move(obj)};
  }

  static constexpr void move(EventStorage &dst, EventStorage &src) {
    dst.heap_allocated_ =
        new T{std::move(*reinterpret_cast<T *>(src.heap_allocated_))};
  }

  static constexpr void const *get(EventStorage const &storage) {
    return storage.heap_allocated_;
  }

  static constexpr void destroy(EventStorage &storage) {
    delete reinterpret_cast<T *>(storage.heap_allocated_);
  }
};

} // namespace details

class Event {
  template <typename T>
  using VTable =
      details::MakeEventVTable<T, sizeof(T) <= sizeof(details::EventStorage)>;

public:
  template <typename E>
  explicit Event(E &&e)
    requires(!std::is_same_v<std::remove_cvref_t<E>, Event>)
  {
    using VTable = VTable<E>;
    VTable::construct(storage_, std::forward<E>(e));

    vtable_ = {
        .move = VTable::move,
        .get = VTable::get,
        .destory = VTable::destroy,
    };
  }

  Event(Event const &) = delete;
  Event &operator=(Event const &) = delete;

  Event(Event &&other) noexcept : vtable_(other.vtable_) {
    vtable_.move(storage_, other.storage_);
  }

  Event &operator=(Event &&other) noexcept {
    Event temp{std::move(other)};
    std::swap(*this, other);
    return *this;
  }

  template <typename T> bool is() const noexcept {
    return vtable_.destory == VTable<T>::destroy;
  }

  template <typename T> T const &as() const {
    assert(is<T>());
    return *reinterpret_cast<T const *>(vtable_.get(storage_));
  }

  ~Event() noexcept { vtable_.destory(storage_); }

private:
  details::EventVTable vtable_{};
  details::EventStorage storage_{};
};
} // namespace ECS::Event
