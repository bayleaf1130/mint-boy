# mintboy root CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)

# Important Vars
set(PROJECT_NAME "mintboy")

project(
    "${PROJECT_NAME}"
    VERSION 0.0.1
    LANGUAGES CXX
)

# Disable insource builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In source builds not allowed, set -b and -h")
endif()

# Options
option(BUILD_TESTS "Build Tests" OFF)

# Packages
find_package(SFML 2.5.1 COMPONENTS system window graphics audio REQUIRED)

add_subdirectory(src)
add_subdirectory(include)

# Maybe add tests
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Targets
add_executable("${PROJECT_NAME}")
target_include_directories("${PROJECT_NAME}" PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries("${PROJECT_NAME}" sfml-graphics sfml-window sfml-system)
install(TARGETS "${PROJECT_NAME}" RUNTIME DESTINATION bin)
