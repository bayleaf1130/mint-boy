# Sources CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)

list(APPEND "INCLUDE_DIRS" "${CMAKE_CURRENT_SOURCE_DIR}/include/core" "${CMAKE_CURRENT_SOURCE_DIR}/include/util")

# Targets
add_executable("${PROJECT_NAME}")
install(TARGETS "${PROJECT_NAME}" RUNTIME DESTINATION bin)

# For threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE Threads::Threads)
# Math lib
target_link_libraries("${PROJECT_NAME}" PRIVATE m)

# Flags
if(MSVC)
    target_compile_options(
        "${PROJECT_NAME}"
        PRIVATE /W4
        $<$<CONFIG:RELEASE>:/O2>
    )
else()
    target_compile_options(
        "${PROJECT_NAME}"
        PRIVATE -Wall
        -Wextra
        -pedantic
        -Werror
        -Wswitch-enum
        -Wcast-align
        -Wpointer-arith
        -Wundef
        -Wnested-externs
        -Wcast-qual
        -Wshadow
        -Wunreachable-code
        -Wfloat-equal
        $<$<CONFIG:RELEASE>:-O2>
    )
endif()

target_compile_definitions("${PROJECT_NAME}" PRIVATE $<$<CONFIG:DEBUG>:MINTBOY_DEBUG>)
set_target_properties("${PROJECT_NAME}" PROPERTIES C_STANDARD 11)

# More things
set_target_properties("${PROJECT_NAME}"
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


target_include_directories("${PROJECT_NAME}" PRIVATE 
    "${INCLUDE_DIRS}"
)

target_sources("${PROJECT_NAME}" PRIVATE 
    src/main.c 
    src/core/cpu.c 
    src/core/bus.c
    src/util/arguments.c
    src/util/common.c
)

