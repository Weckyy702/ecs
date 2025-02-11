cmake_minimum_required(VERSION 3.14)

add_executable(pong
pong.cpp
src/socket.cpp
)

target_include_directories(pong PRIVATE include)
target_link_libraries(pong PRIVATE ecs raylib)
