#pragma once

#include <cstddef>
#include <type_traits>

namespace ECS {
// TODO: find constraints for components
template <typename T>
concept Component = true;

namespace detail {
template <typename, size_t, typename...> struct BitFor;

template <typename T, size_t I, typename Head, typename... Tail>
struct BitFor<T, I, Head, Tail...> {
  constexpr static auto value = BitFor<T, I << 1, Tail...>::value;
};

template <typename T, size_t I, typename... Tail>
struct BitFor<T, I, T, Tail...> {
  constexpr static auto value = I;
};

template <typename T, typename Head, typename... Tail> struct Contains {
  constexpr static bool value =
      Contains<T, Head>::value || Contains<T, Tail...>::value;
};

template <typename T> struct Contains<T, T> {
  constexpr static auto value = true;
};

template <typename T, typename U> struct Contains<T, U> {
  constexpr static auto value = false;
};

} // namespace detail

template <Component T, Component... Cs>
constexpr size_t bit_for = detail::BitFor<T, 1, Cs...>::value;

template <typename T, typename... Us>
constexpr auto contains_v =
    detail::Contains<std::remove_cvref_t<T>, Us...>::value;
} // namespace ECS
