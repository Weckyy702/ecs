#pragma once

namespace ECS {
template <typename T, typename... Cs>
concept System = requires(T s, Cs &...cs) { s(cs...); };

template <typename T, typename... Cs>
concept ConcurrentSystem = requires(const T &s, Cs &...cs) { s(cs...); };
} // namespace ECS
