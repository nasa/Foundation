#.rst:
# FindCoreDX
# ----------
#
# Finds all of the feature variants of the CoreDX DDS libraries.
#
# This will define the following imported targets depending on what features were required::
#
#   CoreDX::C              - The CoreDX C library
#   CoreDX::Ccf            - The CoreDX C library supporting content-filtered topics.
#   CoreDX::Cdyntype       - The CoreDX C library supporting dynamic types.
#   CoreDX::Cdyntypexml    - The CoreDX C library supporting XML-defined dynamic types.
#   CoreDX::Cxx            - The CoreDX C++ library
#   CoreDX::Cxxcf          - The CoreDX C++ library supporting content-filtered topics.
#   CoreDX::Cxxdyntype     - The CoreDX C++ library supporting dynamic types.
#   CoreDX::DdlCompiler    - The CoreDX DDL compiler executable.
#   CoreDX::DdsXml         - The CoreDX XML interface library
#   CoreDX::IsoCxx         - The CoreDX ISO/IEC C++ PSM (i.e., "modern C++") library
#   CoreDX::QosProvider    - The CoreDX QoS provider library
#
# This will also define the following cache variable::
#
#   CoreDX_DDLC  - Location of the CoreDX DDL compiler.

# The following ``set()`` calls will not change the value for the CMake cache
# variables if they have already been initialized by the user via other means
# (e.g., the command line or ``cmake-gui``). See:
# `CMake Set Command <https://cmake.org/cmake/help/latest/command/set.html#set-cache-entry>`_
set (CoreDX_ROOT "$ENV{COREDX_TOP}" CACHE PATH "Path to root of CoreDX installation")
set (CoreDX_TARGET "$ENV{COREDX_TARGET}" CACHE STRING "CoreDX target platform identifier")
set (CoreDX_HOST "$ENV{COREDX_HOST}" CACHE STRING "CoreDX host platform identifier")

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    CoreDX
    REQUIRED_VARS CoreDX_ROOT CoreDX_TARGET CoreDX_HOST
)
set (_isocxx_requested FALSE)
list(FIND CoreDX_FIND_COMPONENTS "IsoCxx" _isocxx_index)
if (NOT _isocxx_index EQUAL -1)
    set (_isocxx_requested TRUE)
    message (STATUS "CoreDX ISO/IEC C++ PSM Requested")
endif ()

find_path(CoreDX_INCLUDE_DIR
    NAMES dds/dds.h
    PATHS
        "${CoreDX_ROOT}/target/include"
    DOC "Path to directory with CoreDX C and classic C++ headers."
    CMAKE_FIND_ROOT_PATH_BOTH
)

find_path(CoreDX_IsoCxx_INCLUDE_DIR NAMES "dds/dds.hpp" PATHS "${CoreDX_ROOT}/target/include/dds_cxx" DOC "Path to CoreDX ISO/IEC C++ PSM headers." CMAKE_FIND_ROOT_PATH_BOTH)

if (NOT CoreDX_INCLUDE_DIR MATCHES "NOTFOUND")
    set (CoreDX_INCLUDE_DIRS "${CoreDX_INCLUDE_DIR}" CACHE STRING "Semicolon-separated list of CoreDX include directories")
    if (_isocxx_requested AND (NOT CoreDX_IsoCxx_INCLUDE_DIR MATCHES "NOTFOUND"))
        list(APPEND CoreDX_INCLUDE_DIRS "${CoreDX_IsoCxx_INCLUDE_DIR}")
    endif ()
endif ()

find_program(CoreDX_DDLC
    NAMES coredx_ddl
    PATHS
        "${CoreDX_ROOT}/host/${CoreDX_HOST}/bin"
)
if (NOT CoreDX_DDLC MATCHES "NOTFOUND")
    add_executable(CoreDX::DdlCompiler IMPORTED)
    set_property(TARGET CoreDX::DdlCompiler PROPERTY IMPORTED_LOCATION "${CoreDX_DDLC}")
endif ()

function (COREDX_CREATE_LIBRARY_TARGET)
    set(_OPTION_ARGS "CHAIN")
    set(_ONE_VALUE_ARGS "COMPONENT" "LIB_BASE_NAME")
    set(_MULTI_VALUE_ARGS "DEPENDENCIES")
    cmake_parse_arguments(CCLT "${_OPTION_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})
    # Do not create any targets if required include directory (or directories) were not found
    if (CoreDX_INCLUDE_DIR MATCHES "NOTFOUND")
        return()
    endif()
    if (_isocxx_requested AND (CoreDX_IsoCxx_INCLUDE_DIR MATCHES "NOTFOUND"))
        return()
    endif()
    find_library(CoreDX_${CCLT_COMPONENT}_LIBRARY NAMES ${CCLT_LIB_BASE_NAME} PATHS "${CoreDX_ROOT}/target/${CoreDX_TARGET}/lib" DOC "Location of CoreDX ${CCLT_COMPONENT} library." CMAKE_FIND_ROOT_PATH_BOTH)
    find_library(CoreDX_${CCLT_COMPONENT}_DBG_ADD_ON_LIBRARY NAMES ${CCLT_LIB_BASE_NAME}_log PATHS "${CoreDX_ROOT}/target/${CoreDX_TARGET}/lib" DOC "Location of CoreDX ${CCLT_COMPONENT} debug add-on library." CMAKE_FIND_ROOT_PATH_BOTH)
    if (CCLT_CHAIN)
        if (NOT CoreDX_${CCLT_COMPONENT}_LIBRARY MATCHES "NOTFOUND")
            add_library(CoreDX::${CCLT_COMPONENT} UNKNOWN IMPORTED)
            # If "_log" add-on is available, create chain from baseline to it,
            # otherwise, just use baseline
            if ((CMAKE_BUILD_TYPE MATCHES "Debug") AND (NOT CoreDX_${CCLT_COMPONENT}_DBG_ADD_ON_LIBRARY MATCHES "NOTFOUND"))
                add_library(CoreDX::${CCLT_COMPONENT}Base UNKNOWN IMPORTED)
                set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY IMPORTED_LOCATION "${CoreDX_${CCLT_COMPONENT}_DBG_ADD_ON_LIBRARY}")
                set_property(TARGET CoreDX::${CCLT_COMPONENT}Base PROPERTY IMPORTED_LOCATION "${CoreDX_${CCLT_COMPONENT}_LIBRARY}")
                set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY INTERFACE_LINK_LIBRARIES "CoreDX::${CCLT_COMPONENT}Base")
            else ()
                set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY IMPORTED_LOCATION "${CoreDX_${CCLT_COMPONENT}_LIBRARY}")
            endif ()
        endif ()
    else ()
        if (CMAKE_BUILD_TYPE MATCHES "Debug")
            # Pick "_log" variant if available, otherwise baseline.
            if (NOT CoreDX_${CCLT_COMPONENT}_LIBRARY MATCHES "NOTFOUND")
                add_library(CoreDX::${CCLT_COMPONENT} UNKNOWN IMPORTED)
                if (NOT CoreDX_${CCLT_COMPONENT}_DBG_ADD_ON_LIBRARY MATCHES "NOTFOUND")
                    set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY IMPORTED_LOCATION "${CoreDX_${CCLT_COMPONENT}_DBG_ADD_ON_LIBRARY}")
                else ()
                    set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY IMPORTED_LOCATION "${CoreDX_${CCLT_COMPONENT}_LIBRARY}")
                endif ()
            endif ()
        else ()
            # Pick baseline variant.
            if (NOT CoreDX_${CCLT_COMPONENT}_LIBRARY MATCHES "NOTFOUND")
                add_library(CoreDX::${CCLT_COMPONENT} UNKNOWN IMPORTED)
                set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY IMPORTED_LOCATION "${CoreDX_${CCLT_COMPONENT}_LIBRARY}")
            endif ()
        endif ()
    endif ()
    if (TARGET CoreDX::${CCLT_COMPONENT})
        if (DEFINED CCLT_DEPENDENCIES)
            # This assumes the dependencies are all internal to CoreDX. If
            # this were to evolve so that dependencies external to CoreDX
            # must be added as well, the logic will need to be revised.
            list(TRANSFORM CCLT_DEPENDENCIES PREPEND "CoreDX::")
            set_property(TARGET CoreDX::${CCLT_COMPONENT} APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${CCLT_DEPENDENCIES}")
        else ()
            set_property(TARGET CoreDX::${CCLT_COMPONENT} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CoreDX_INCLUDE_DIRS}")
        endif ()
    endif ()
    mark_as_advanced(CoreDX_${CCLT_COMPONENT}_LIBRARY CoreDX_${CCLT_COMPONENT}_DBG_ADD_ON_LIBRARY)
endfunction ()

coredx_create_library_target(COMPONENT C LIB_BASE_NAME dds)
coredx_create_library_target(COMPONENT Ccf LIB_BASE_NAME dds_cf CHAIN DEPENDENCIES C)
coredx_create_library_target(COMPONENT Cdyntype LIB_BASE_NAME dds_dyntype CHAIN DEPENDENCIES C)
coredx_create_library_target(COMPONENT Cdyntypexml LIB_BASE_NAME dds_dyntype_xml DEPENDENCIES Cdyntype)
coredx_create_library_target(COMPONENT Cxx LIB_BASE_NAME dds_cpp DEPENDENCIES C)
coredx_create_library_target(COMPONENT Cxxcf LIB_BASE_NAME dds_cpp_cf DEPENDENCIES Cxx Ccf)
coredx_create_library_target(COMPONENT Cxxdyntype LIB_BASE_NAME dds_cpp_dyntype DEPENDENCIES Cxx Cdyntype)
coredx_create_library_target(COMPONENT DdsXml LIB_BASE_NAME dds_xml CHAIN DEPENDENCIES C)
coredx_create_library_target(COMPONENT QosProvider LIB_BASE_NAME dds_qos_provider DEPENDENCIES C)
coredx_create_library_target(COMPONENT IsoCxx LIB_BASE_NAME dds_cxx DEPENDENCIES Cdyntypexml Cdyntype QosProvider DdsXml Ccf C)

foreach (_req_component IN LISTS CoreDX_FIND_COMPONENTS)
    if (TARGET CoreDX::${_req_component})
        message(STATUS "Found CoreDX component ``${_req_component}``")
    elseif (CoreDX_FIND_REQUIRED_${_req_component})
        message(FATAL_ERROR "Could not find required CoreDX component ``${_req_component}``")
    endif ()
endforeach ()

mark_as_advanced(CoreDX_INCLUDE_DIR CoreDX_IsoCxx_INCLUDE_DIR)

# vim: set ts=4 sw=4 expandtab:

