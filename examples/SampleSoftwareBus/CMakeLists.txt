# =============================================================================
# CMake project configuration file for a Software Bus skeleton definition.
# =============================================================================
cmake_minimum_required(VERSION 3.16)

project(SampleSoftwareBus LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

include(CMakePackageConfigHelpers)
include(TargetIDLSources)

if (WITH_DDS STREQUAL "CoreDX")
    set (DDS_PROVIDER_COMP "Cxxcf")
    set (DDS_PROVIDER_LIB "CoreDX::Cxxcf")
elseif (WITH_DDS STREQUAL "OpenSplice")
    set (DDS_PROVIDER_COMP "Sacpp")
    set (DDS_PROVIDER_LIB "OpenSplice::Sacpp")
endif ()

find_package(${WITH_DDS} REQUIRED COMPONENTS ${DDS_PROVIDER_COMP})

# -----------------------------------------------------------------------------
# Build plan for the software bus library
# -----------------------------------------------------------------------------
add_library(${PROJECT_NAME} STATIC
    "<insert topic configuration C++ files to build>"
)

target_idl_sources(${PROJECT_NAME}
    DDS_PROVIDER ${WITH_DDS}
    IDL_SOURCES "<insert IDL file path>"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/${WITH_DDS}>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
        ${OpenSplice_INCLUDE_DIRS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VERSION ${PROJECT_VERSION})

# -----------------------------------------------------------------------------
# Install/Deploy plan for the software bus library
# -----------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

get_target_property(DDS_HEADERS ${PROJECT_NAME} IDL_LANG_BINDING_HEADERS)

install(
    FILES
        "<insert topic configuration header files>"
        ${DDS_HEADERS}
    DESTINATION include
    COMPONENT Devel
)

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
    "cmake/SampleSoftwareBusConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
    @ONLY
)

set(ConfigPackageLocation "lib/cmake/${PROJECT_NAME}")

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${ConfigPackageLocation}
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION
        ${ConfigPackageLocation}
    COMPONENT Devel
)

# vim: set ts=4 sw=4 expandtab:
