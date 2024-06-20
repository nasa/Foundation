cmake_minimum_required(VERSION "3.16")

project(NcursesKit VERSION "1.0.0" LANGUAGES CXX)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

include(GenerateExportHeader)

add_library(${PROJECT_NAME} STATIC
    "src/Control.cpp"
    "src/OnOffIndicator.cpp"
    "src/ToggleButton.cpp"
    "src/VerticalGauge.cpp"
    "src/VerticalSlider.cpp"
    "src/Widget.cpp"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
        ${CURSES_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${CURSES_LIBRARIES})

generate_export_header(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(
    FILES
        "src/Control.hh"
        "src/Dimensions.hh"
        "src/FocusReceiver.hh"
        "src/InputReceiver.hh"
        "src/OnOffIndicator.hh"
        "src/ScreenPosition.hh"
        "src/ToggleButton.hh"
        "src/VerticalGauge.hh"
        "src/VerticalSlider.cpp"
        "src/Widget.hh"
        "${CMAKE_CURRENT_BINARY_DIR}/ncurseskit_export.h"
    DESTINATION include
    COMPONENT Devel
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(
    EXPORT ${PROJECT_NAME}Targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
)

configure_file(
    "cmake/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    COPYONLY
)

set(ConfigPackageLocation "lib/cmake/${PROJECT_NAME}")

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${ConfigPackageLocation}
)

install(
    FILES
        "cmake/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        ${ConfigPackageLocation}
    COMPONENT Devel
)

# vim: set ts=4 sw=4 expandtab:
