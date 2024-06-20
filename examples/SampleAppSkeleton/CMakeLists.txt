# =============================================================================
# CMake project file for the Foundation sample application skeleton
# =============================================================================
cmake_minimum_required(VERSION 3.12)

project(SampleApp LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

find_package(Foundation REQUIRED CoreKit)

add_executable(${PROJECT_NAME}
    "src/main.cpp"
    "src/MyAppDelegate.cpp"
)

target_link_libraries(${PROJECT_NAME} Foundation::CoreKit)

# =============================================================================
# Compile command information pseudo-target
# =============================================================================
if (UNIX AND CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        compile_commands ALL
        COMMAND
            "${CMAKE_COMMAND}"
            "-E"
            "create_symlink"
            "${PROJECT_BINARY_DIR}/compile_commands.json"
            "${PROJECT_SOURCE_DIR}/compile_commands.json"
        DEPENDS
            "${PROJECT_BINARY_DIR}/compile_commands.json"
    )
endif ()

# vim: set ts=4 sw=4 expandtab:
