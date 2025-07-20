cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

project(dug LANGUAGES C)

file(GLOB_RECURSE C_SOURCE_FILES "src/*.c" "src/*.h")

add_executable(dug ${C_SOURCE_FILES})
