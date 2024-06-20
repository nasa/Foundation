# =============================================================================
# CMake project file for the Foundation sample application skeleton
# =============================================================================
cmake_minimum_required(VERSION 3.12)

project(ControlPanel VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

find_package(Foundation REQUIRED COMPONENTS CoreKit DdsKit)
find_package(NcursesKit REQUIRED)
find_package(MySoftwareBus REQUIRED)

add_executable(${PROJECT_NAME}
    "src/CommandDispatch.cpp"
    "src/FrontPanel.cpp"
    "src/main.cpp"
    "src/MyAppDelegate.cpp"
    "src/UserInputEventDispatch.cpp"
    "src/UserInputSource.cpp"
)

target_link_libraries(${PROJECT_NAME} Foundation::DdsKit Foundation::CoreKit NcursesKit::NcursesKit MySoftwareBus::MySoftwareBus)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/src")

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
