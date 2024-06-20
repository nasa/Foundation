# Licensed for U.S. Government use only.
#.rst:
# FindOpenSplice
# --------------
#
# Finds all of the feature variants of the OpenSplice DDS libraries.
#
# This will define the following variables::
#
#   OpenSplice_FOUND        - TRUE if OpenSplice was found
#   OpenSplice_INCLUDE_DIRS - List of directories to add to dependent target include path
#   OpenSplice_LIBRARIES    - List of libraries to add to dependent target
#   OpenSplice_IDLPP        - Path to IDL compiler executable
#
# and some of the following imported targets depending on what features were required::
#
#   OpenSplice::Sac        - The OpenSplice stand-alone C library
#   OpenSplice::Sacpp      - The OpenSplice stand-alone C++ library.
#   OpenSplice::Isocpp     - The OpenSplice ISO C++ binding library.
#   OpenSplice::Isocpp2    - The OpenSplice ISO C++ v2 binding library.

if(NOT IS_DIRECTORY "$ENV{OSPL_HOME}")
    message(WARNING "The 'OSPL_HOME' environment variable is not properly configured. IDL compilation may fail.")
endif()

if(NOT IS_DIRECTORY "$ENV{OSPL_TMPL_PATH}")
    message(WARNING "The 'OSPL_TMPL_PATH' environment variable is not properly configured. IDL compilation may fail.")
endif()

if(NOT OpenSplice_ROOT_DIR)
    set(OpenSplice_ROOT_DIR "$ENV{OSPL_HOME}"
        CACHE PATH
        "Root directory to OpenSplice HDE installation."
    )
else()
    set(OpenSplice_ROOT_DIR "${OpenSplice_ROOT_DIR}"
        CACHE PATH
        "Root directory to OpenSplice HDE installation."
        FORCE
    )
endif()

find_program(OpenSplice_IDLPP
    idlpp
    PATHS "${OpenSplice_ROOT_DIR}/bin"
)
list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
    "OpenSplice_IDLPP"
)

if(EXISTS "${OpenSplice_ROOT_DIR}/include")
    set(OpenSplice_Root_INCLUDE_DIR "${OpenSplice_ROOT_DIR}/include")
endif()
list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
    "OpenSplice_Root_INCLUDE_DIR"
)

find_path(OpenSplice_Sys_INCLUDE_DIR
    NAMES c_base.h
    PATHS
        ${OpenSplice_Root_INCLUDE_DIR}/sys
)
list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
    "OpenSplice_Sys_INCLUDE_DIR"
)

find_library(OpenSplice_Kernel_LIBRARY
    NAMES ddskernel
    PATHS
        ${OpenSplice_ROOT_DIR}/lib
)
list(APPEND OpenSplice_REQUIRED_LIB_VARS
    "OpenSplice_Kernel_LIBRARY"
)

if(OpenSplice_FIND_REQUIRED_Sac)
    find_path(OpenSplice_Sac_INCLUDE_DIR
        NAMES dds_dcps.h
        PATHS
            ${OpenSplice_Root_INCLUDE_DIR}/dcps/C/SAC
    )
    find_library(OpenSplice_Sac_LIBRARY
        NAMES dcpssac
        PATHS
            ${OpenSplice_ROOT_DIR}/lib
    )
    list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
        "OpenSplice_Sac_INCLUDE_DIR"
    )
    list(APPEND OpenSplice_REQUIRED_LIB_VARS
        "OpenSplice_Sac_LIBRARY"
    )
elseif(OpenSplice_FIND_REQUIRED_Sacpp)
    find_path(OpenSplice_Sacpp_INCLUDE_DIR
        NAMES dds_dcps.h
        PATHS
            ${OpenSplice_Root_INCLUDE_DIR}/dcps/C++/SACPP
    )
    find_library(OpenSplice_Sacpp_LIBRARY
        NAMES dcpssacpp
        PATHS
            ${OpenSplice_ROOT_DIR}/lib
    )
    list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
        "OpenSplice_Sacpp_INCLUDE_DIR"
    )
    list(APPEND OpenSplice_REQUIRED_LIB_VARS
        "OpenSplice_Sacpp_LIBRARY"
    )
elseif(OpenSplice_FIND_REQUIRED_Isocpp)
    find_path(OpenSplice_Isocpp_INCLUDE_DIR
        NAMES dds/dds.hpp
        PATHS
            ${OpenSplice_Root_INCLUDE_DIR}/dcps/C++/isocpp
    )
    find_library(OpenSplice_Isocpp_LIBRARY
        NAMES dcpsisocpp
        PATHS
            ${OpenSplice_ROOT_DIR}/lib
    )
    list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
        "OpenSplice_Isocpp_INCLUDE_DIR"
    )
    list(APPEND OpenSplice_REQUIRED_LIB_VARS
        "OpenSplice_Isocpp_LIBRARY"
    )
elseif(OpenSplice_FIND_REQUIRED_Isocpp2)
    find_path(OpenSplice_Isocpp2_INCLUDE_DIR
        NAMES dds/dds.hpp
        PATHS
            ${OpenSplice_Root_INCLUDE_DIR}/dcps/C++/isocpp2
    )
    find_library(OpenSplice_Isocpp2_LIBRARY
        NAMES dcpsisocpp2
        PATHS
            ${OpenSplice_ROOT_DIR}/lib
    )
    list(APPEND OpenSplice_REQUIRED_INCLUDE_DIR_VARS
        "OpenSplice_Isocpp2_INCLUDE_DIR"
    )
    list(APPEND OpenSplice_REQUIRED_LIB_VARS
        "OpenSplice_Isocpp2_LIBRARY"
    )
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(OpenSplice
    FOUND_VAR OpenSplice_FOUND
    REQUIRED_VARS
        ${OpenSplice_REQUIRED_INCLUDE_DIR_VARS}
        ${OpenSplice_REQUIRED_LIB_VARS}
)

if(OpenSplice_FOUND)
    # Classic approach of defining variables
    foreach(A_INCLUDE_DIR_VAR IN LISTS OpenSplice_REQUIRED_INCLUDE_DIR_VARS)
        list(APPEND OpenSplice_INCLUDE_DIRS ${${A_INCLUDE_DIR_VAR}})
    endforeach()
    foreach(A_LIB_VAR IN LISTS OpenSplice_REQUIRED_LIB_VARS)
        list(APPEND OpenSplice_LIBRARIES ${${A_LIB_VAR}})
    endforeach()

    # Modern approach of defining imported targets
    if(NOT TARGET OpenSplice::Kernel)
        add_library(OpenSplice::Kernel UNKNOWN IMPORTED)
        set_target_properties(OpenSplice::Kernel PROPERTIES
            IMPORTED_LOCATION ${OpenSplice_Kernel_LIBRARY}
        )
        set_property(TARGET OpenSplice::Kernel PROPERTY
            INTERFACE_INCLUDE_DIRECTORIES
                ${OpenSplice_Root_INCLUDE_DIR}
                ${OpenSplice_Sys_INCLUDE_DIR}
        )
    endif()
    if(OpenSplice_FIND_REQUIRED_Sac AND NOT TARGET OpenSplice::Sac)
        add_library(OpenSplice::Sac UNKNOWN IMPORTED)
        set_target_properties(OpenSplice::Sac PROPERTIES
            IMPORTED_LOCATION ${OpenSplice_Sac_LIBRARY}
            INTERFACE_LINK_LIBRARIES OpenSplice::Kernel
            INTERFACE_INCLUDE_DIRECTORIES ${OpenSplice_Sac_INCLUDE_DIR}
        )
    endif()
    if(OpenSplice_FIND_REQUIRED_Sacpp AND NOT TARGET OpenSplice::Sacpp)
        add_library(OpenSplice::Sacpp UNKNOWN IMPORTED)
        set_target_properties(OpenSplice::Sacpp PROPERTIES
            IMPORTED_LOCATION ${OpenSplice_Sacpp_LIBRARY}
            INTERFACE_LINK_LIBRARIES OpenSplice::Kernel
            INTERFACE_INCLUDE_DIRECTORIES ${OpenSplice_Sacpp_INCLUDE_DIR}
        )
    endif()
    if(OpenSplice_FIND_REQUIRED_Isocpp AND NOT TARGET OpenSplice::Isocpp)
        add_library(OpenSplice::Isocpp UNKNOWN IMPORTED)
        set_target_properties(OpenSplice::Isocpp PROPERTIES
            IMPORTED_LOCATION ${OpenSplice_Isocpp_LIBRARY}
            INTERFACE_LINK_LIBRARIES OpenSplice::Kernel
            INTERFACE_INCLUDE_DIRECTORIES ${OpenSplice_Isocpp_INCLUDE_DIR}
        )
    endif()
    if(OpenSplice_FIND_REQUIRED_Isocpp2 AND NOT TARGET OpenSplice::Isocpp2)
        add_library(OpenSplice::Isocpp2 UNKNOWN IMPORTED)
        set_target_properties(OpenSplice::Isocpp2 PROPERTIES
            IMPORTED_LOCATION ${OpenSplice_Isocpp2_LIBRARY}
            INTERFACE_LINK_LIBRARIES OpenSplice::Kernel
            INTERFACE_INCLUDE_DIRECTORIES ${OpenSplice_Isocpp2_INCLUDE_DIR}
        )
    endif()
endif()

mark_as_advanced(
    OpenSplice_Root_INCLUDE_DIR
    OpenSplice_Sys_INCLUDE_DIR
    OpenSplice_Sac_INCLUDE_DIR
    OpenSplice_Sacpp_INCLUDE_DIR
    OpenSplice_Isocpp_INCLUDE_DIR
    OpenSplice_Isocpp2_INCLUDE_DIR
    OpenSplice_Kernel_LIBRARY
    OpenSplice_Sac_LIBRARY
    OpenSplice_Sacpp_LIBRARY
    OpenSplice_Isocpp_LIBRARY
    OpenSplice_Isocpp2_LIBRARY
)
