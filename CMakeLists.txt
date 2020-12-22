# mintboy root CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)

# Important Vars
set(PROJECT_NAME "mintboy")

project(
    "${PROJECT_NAME}"
    VERSION 0.0.1
    LANGUAGES C
)

# Disable insource builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In source builds not allowed, set -B and -H")
endif()

# Options
option(BUILD_TESTS "Build Tests" OFF)

# Projects
add_subdirectory(mintboy)

# Maybe add tests
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()