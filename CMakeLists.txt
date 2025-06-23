cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

project(hiro LANGUAGES C)

file(GLOB_RECURSE C_SOURCE_FILES "src/*.c" "src/*.h")

add_executable(hiro ${C_SOURCE_FILES})
