# Sources CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)

# Targets
add_executable("${PROJECT_NAME}")
install(TARGETS "${PROJECT_NAME}" RUNTIME DESTINATION bin)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries("${PROJECT_NAME}" PRIVATE Threads::Threads)

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

target_compile_definitions("${PROJECT_NAME}" PRIVATE $<$<CONFIG:DEBUG>:DEBUG>)
set_target_properties("${PROJECT_NAME}" PROPERTIES C_STANDARD 11)

# More things
set_target_properties("${PROJECT_NAME}"
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)


target_include_directories("${PROJECT_NAME}" PRIVATE 
    "include"
)

target_sources("${PROJECT_NAME}" PRIVATE 
    src/main.c 
    src/cpu/cpu.c 
    src/util/log.c
    src/util/arguments.c
    src/util/common.c
)

