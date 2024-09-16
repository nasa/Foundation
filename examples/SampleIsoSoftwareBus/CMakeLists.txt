# ===========================================================================
# CMake project configuration file for an ISO C++ DDS PSM Software Bus
# ===========================================================================
cmake_minimum_required(VERSION "3.16")

project(SampleIsoSoftwareBus VERSION "1.0.0" LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

include(CMakePackageConfigHelpers)
include("${WITH_DDS}-build")

if (WITH_DDS STREQUAL "CycloneDDS-CXX")
    find_package(iceoryx_binding_c REQUIRED)
    find_package("${WITH_DDS}" REQUIRED)
elseif (WITH_DDS STREQUAL "CoreDX")
    find_package("${WITH_DDS}" REQUIRED COMPONENTS IsoCxx DdlCompiler)
endif ()


# -----------------------------------------------------------------------------
# Build plan for the software bus library
# -----------------------------------------------------------------------------
create_sb_library_targets(
    TARGET "${PROJECT_NAME}"
    IDL_FILES
        "<insert IDL file path>"
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

# -----------------------------------------------------------------------------
# Generate amalgam header file that includes all provider-generated headers.
# -----------------------------------------------------------------------------
get_target_property(HEADERS_TO_INSTALL "${PROJECT_NAME}" IDL_LANG_BINDING_HEADERS)

foreach (_A_HEADER IN LISTS HEADERS_TO_INSTALL)
    get_filename_component(_HEADER_FILE_NAME "${_A_HEADER}" NAME)
    set(INCLUDE_STATEMENTS "${INCLUDE_STATEMENTS}#include <${_HEADER_FILE_NAME}>\n")
endforeach ()

configure_file("src/${PROJECT_NAME}.h.in" "${PROJECT_NAME}.h" @ONLY)

# -----------------------------------------------------------------------------
# Install/Deploy plan for the software bus library
# -----------------------------------------------------------------------------
install(
    TARGETS "${PROJECT_NAME}"
    EXPORT "${PROJECT_NAME}Targets"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include/${PROJECT_NAME}"
)

list(APPEND HEADERS_TO_INSTALL "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.h")

install(
    FILES ${HEADERS_TO_INSTALL}
    DESTINATION "include/${PROJECT_NAME}"
    COMPONENT "Devel"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY  SameMinorVersion
)

export(
    EXPORT "${PROJECT_NAME}Targets"
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake"
    NAMESPACE "${PROJECT_NAME}::"
)

configure_file(
    "cmake/SampleIsoSoftwareBusConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    @ONLY
)

set (ConfigPackageLocation "lib/cmake/${PROJECT_NAME}")

install(
    EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE "${PROJECT_NAME}::"
    DESTINATION "${ConfigPackageLocation}"
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION "${ConfigPackageLocation}"
    COMPONENT Devel
)

# vim: set ts=4 sw=4 expandtab:
