cmake_minimum_required(VERSION 3.25)
project(calendar_engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib/glfw")
find_package(glfw3 3.5 REQUIRED)

add_library(glad STATIC lib/glad/src/glad.c)
target_include_directories(glad PUBLIC lib/glad/include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(calendar ${SOURCES})
target_link_libraries(calendar PRIVATE glfw glad)