#pragma once

#include <any>

namespace ecs::event {

class Event {
public:
  template <typename E> explicit Event(E &&e) : data_{std::forward<E>(e)} {}

  template <typename T> bool is() const noexcept {
    return data_.type() == typeid(T);
  }

  template <typename T> T const &as() const {
    return std::any_cast<T const &>(data_);
  }

private:
  std::any data_;
};
} // namespace ecs::event
