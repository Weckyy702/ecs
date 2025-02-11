cmake_minimum_required(VERSION 3.14)

project(ECS VERSION 0.0.1 LANGUAGES CXX)

file(GLOB_RECURSE ECS_SOURCES LIST_DIRECTORIES false src/*.cpp)
add_library(ecs ${ECS_SOURCES})

target_include_directories(ecs PUBLIC include)
target_compile_features(ecs PUBLIC cxx_std_26)

target_compile_options(ecs PRIVATE -Wall -Wextra -Wconversion -Wpedantic)

add_subdirectory(examples)
