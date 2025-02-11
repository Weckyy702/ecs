cmake_minimum_required(VERSION 3.14)

add_executable(MainExample main.cpp)

target_link_libraries(MainExample PRIVATE ecs fmt)
target_compile_options(MainExample PRIVATE -Wall -Wextra -Wconversion -Wpedantic)
