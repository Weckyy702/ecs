# ECS in Modern C++

Entity Component System (ECS) is a design pattern that separates data and behaviour.
It's commonly used in games because of its great performance.

Entities hold Components, which are plain old data (POD) objects.
Systems operate on Components, providing behaviour.
