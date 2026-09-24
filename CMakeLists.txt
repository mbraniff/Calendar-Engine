cmake_minimum_required(VERSION 3.25)
project(calendar_engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build configuration: where it runs (DEV|PI) and which GL API it uses (CORE|ES).
set(TARGET_PROFILE DEV CACHE STRING "Target profile: DEV (windowed) or PI (fullscreen kiosk)")
set_property(CACHE TARGET_PROFILE PROPERTY STRINGS DEV PI)
set(GL_API CORE CACHE STRING "GL API: CORE (desktop OpenGL 3.3 core) or ES (OpenGL ES 3.1)")
set_property(CACHE GL_API PROPERTY STRINGS CORE ES)

set(_target_profiles DEV PI)
set(_gl_apis CORE ES)
if(NOT TARGET_PROFILE IN_LIST _target_profiles)
    message(FATAL_ERROR "TARGET_PROFILE must be DEV or PI (got '${TARGET_PROFILE}')")
endif()
if(NOT GL_API IN_LIST _gl_apis)
    message(FATAL_ERROR "GL_API must be CORE or ES (got '${GL_API}')")
endif()
if(TARGET_PROFILE STREQUAL "PI" AND GL_API STREQUAL "CORE")
    message(WARNING "TARGET_PROFILE=PI with GL_API=CORE: the Raspberry Pi 4 has no "
                    "OpenGL 3.3 core context. This binary will fail to create a context on the Pi; use GL_API=ES.")
endif()
message(STATUS "Configuration: TARGET_PROFILE=${TARGET_PROFILE} GL_API=${GL_API}")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/lib/glfw")
find_package(glfw3 3.4 REQUIRED)

if(GL_API STREQUAL "CORE")
    add_library(glad STATIC third_party/glad/src/glad.c)
    target_include_directories(glad PUBLIC third_party/glad/include)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLES REQUIRED IMPORTED_TARGET glesv2 egl)
endif()

add_executable(calendar
    src/app/main.cpp
    src/app/stb_image.cpp
    src/platform/platform_glfw.cpp
)
target_include_directories(calendar PRIVATE src)
target_compile_definitions(calendar PRIVATE
    APP_PROFILE_${TARGET_PROFILE}
    APP_GL_${GL_API}
)
target_link_libraries(calendar PRIVATE glfw)

if(GL_API STREQUAL "CORE")
    target_link_libraries(calendar PRIVATE glad)
else()
    target_link_libraries(calendar PRIVATE PkgConfig::GLES)
endif()
