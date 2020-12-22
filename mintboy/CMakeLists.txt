# Sources CMakeLists.txt

cmake_minimum_required(VERSION 3.13.4)

# Targets
add_executable("${PROJECT_NAME}")
install(TARGETS "${PROJECT_NAME}" RUNTIME DESTINATION bin)

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
)

