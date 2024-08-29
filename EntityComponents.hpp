#pragma once

#include "Component.hpp"
namespace ECS {
namespace detail {
template <Component C> class EntityComponentImpl {};
} // namespace detail
template <Component... Cs>
class EntityComponents : private detail::EntityComponentImpl<Cs>... {
public:
private:
};
} // namespace ECS
