cmake_minimum_required(VERSION 3.14)

add_executable(EventExample events.cpp)
target_link_libraries(EventExample ecs)
