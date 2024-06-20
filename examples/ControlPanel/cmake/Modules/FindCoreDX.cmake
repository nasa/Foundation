# Licensed for U.S. Government use only.
#.rst:
# FindCoreDX
# ----------
#
# Finds all of the feature variants of the CoreDX DDS libraries.
#
# This will define the following variables::
#
#   CoreDX_FOUND        - TRUE if CoreDX was found
#   CoreDX_INCLUDE_DIRS - List of directories to add to dependent target include path
#   CoreDX_LIBRARIES    - List of libraries to add to dependent target
#   CoreDX_DDLC         - Path to IDL compiler executable
#
# and some of the following imported targets depending on what features were required::
#
#   CoreDX::C              - The CoreDX C library
#   CoreDX::Ccf            - The CoreDX C library supporting content-filtered topics.
#   CoreDX::Cdyntype       - The CoreDX C library supporting dynamic types.
#   CoreDX::Cdyntypexml    - The CoreDX C library supporting XML-defined dynamic types.
#   CoreDX::Cxx            - The CoreDX C++ library
#   CoreDX::Cxxcf          - The CoreDX C++ library supporting content-filtered topics.
#   CoreDX::Cxxdyntype     - The CoreDX C++ library supporting dynamic types.

find_path(CoreDX_INCLUDE_DIR
    NAMES dds/dds.h
    PATHS
        ${CoreDX_ROOT_DIR}/target/include
        $ENV{COREDX_TOP}/target/include
    CMAKE_FIND_ROOT_PATH_BOTH
)

find_program(CoreDX_DDLC
    NAMES coredx_ddl
    PATHS ${CoreDX_ROOT_DIR}/host/${CoreDX_HOST}/bin
    $ENV{COREDX_TOP}/host/$ENV{COREDX_HOST}/bin
)

if(DEFINED CoreDX_ROOT_DIR AND DEFINED CoreDX_TARGET)
    list(APPEND COREDX_LIBRARY_SEARCH_PATHS ${CoreDX_ROOT_DIR}/target/${CoreDX_TARGET}/lib)
endif()
list(APPEND COREDX_LIBRARY_SEARCH_PATHS $ENV{COREDX_TOP}/target/$ENV{COREDX_TARGET}/lib)

macro (COREDX_FIND_DEBUG_AND_RELEASE_LIB BASENAME VAR_FRAG)
    find_library(CoreDX_${VAR_FRAG}_LIBRARY_RELEASE
        NAMES ${BASENAME}
        PATHS ${COREDX_LIBRARY_SEARCH_PATHS}
        CMAKE_FIND_ROOT_PATH_BOTH
    )
    find_library(CoreDX_${VAR_FRAG}_LIBRARY_DEBUG
        NAMES ${BASENAME}_log
        PATHS ${COREDX_LIBRARY_SEARCH_PATHS}
        CMAKE_FIND_ROOT_PATH_BOTH
    )
endmacro (COREDX_FIND_DEBUG_AND_RELEASE_LIB)

coredx_find_debug_and_release_lib(dds C)
coredx_find_debug_and_release_lib(dds_cf Ccf)
coredx_find_debug_and_release_lib(dds_dyntype Cdyntype)
find_library(CoreDX_Cdyntypexml_LIBRARY
    NAMES dds_dyntype_xml
    PATHS ${COREDX_LIBRARY_SEARCH_PATHS}
    CMAKE_FIND_ROOT_PATH_BOTH
)
coredx_find_debug_and_release_lib(dds_cpp Cxx)
coredx_find_debug_and_release_lib(dds_cpp_cf Cxxcf)
coredx_find_debug_and_release_lib(dds_cpp_dyntype Cxxdyntype)

list(APPEND CoreDX_REQUIRED_VARS CoreDX_INCLUDE_DIR)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CoreDX_REQUIRED_VARS CoreDX_C_LIBRARY_DEBUG)
    if(CoreDX_FIND_REQUIRED_Cdyntypexml)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cdyntypexml_LIBRARY)
    endif()
    if(CoreDX_FIND_REQUIRED_Cdyntype)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cdyntype_LIBRARY_DEBUG)
    endif()
    if(CoreDX_FIND_REQUIRED_Ccf)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Ccf_LIBRARY_DEBUG)
    endif()
    if(CoreDX_FIND_REQUIRED_Cxxdyntype)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cxxdyntype_LIBRARY_DEBUG CoreDX_Cxx_LIBRARY_DEBUG CoreDX_Cdyntype_LIBRARY_DEBUG)
    endif()
    if(CoreDX_FIND_REQUIRED_Cxxcf)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cxxcf_LIBRARY_DEBUG CoreDX_Cxx_LIBRARY_DEBUG CoreDX_Ccf_LIBRARY_DEBUG)
    endif()
    if(CoreDX_FIND_REQUIRED_Cxx)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cxx_LIBRARY_DEBUG)
    endif()
else()
    list(APPEND CoreDX_REQUIRED_VARS CoreDX_C_LIBRARY_RELEASE)
    if(CoreDX_FIND_REQUIRED_Cdyntypexml)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cdyntypexml_LIBRARY)
    endif()
    if(CoreDX_FIND_REQUIRED_Cdyntype)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cdyntype_LIBRARY_RELEASE)
    endif()
    if(CoreDX_FIND_REQUIRED_Ccf)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Ccf_LIBRARY_RELEASE)
    endif()
    if(CoreDX_FIND_REQUIRED_Cxxdyntype)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cxxdyntype_LIBRARY_RELEASE CoreDX_Cxx_LIBRARY_RELEASE CoreDX_Cdyntype_LIBRARY_RELEASE)
    endif()
    if(CoreDX_FIND_REQUIRED_Cxxcf)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cxxcf_LIBRARY_RELEASE CoreDX_Cxx_LIBRARY_RELEASE CoreDX_Ccf_LIBRARY_RELEASE)
    endif()
    if(CoreDX_FIND_REQUIRED_Cxx)
        list(APPEND CoreDX_REQUIRED_VARS CoreDX_Cxx_LIBRARY_RELEASE)
    endif()
endif()

list(REMOVE_DUPLICATES CoreDX_REQUIRED_VARS)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args(CoreDX
    FOUND_VAR CoreDX_FOUND
    REQUIRED_VARS ${CoreDX_REQUIRED_VARS}
)

if (CoreDX_FOUND)
    # Classic approach of defining variables
    list(GET CoreDX_REQUIRED_VARS 0 CoreDX_INCLUDE_DIRS_VAR)
    list(REMOVE_AT CoreDX_REQUIRED_VARS 0)
    set(CoreDX_INCLUDE_DIRS ${${CoreDX_INCLUDE_DIRS_VAR}})
    unset(CoreDX_INCLUDE_DIRS_VAR)
    foreach(A_CoreDX_LIB_VAR IN LISTS CoreDX_REQUIRED_VARS)
        list(APPEND CoreDX_LIBRARIES ${${A_CoreDX_LIB_VAR}})
    endforeach(A_CoreDX_LIB_VAR)
    unset(A_CoreDX_LIB_VAR)

    # Modern approach of defining imported targets
    if (NOT TARGET CoreDX::C)
        add_library(CoreDX::C UNKNOWN IMPORTED)
    endif ()
    if (CoreDX_C_LIBRARY_RELEASE)
        set_property(TARGET CoreDX::C APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(CoreDX::C PROPERTIES IMPORTED_LOCATION_RELEASE "${CoreDX_C_LIBRARY_RELEASE}")
    endif ()
    if (CoreDX_C_LIBRARY_DEBUG)
        set_property(TARGET CoreDX::C APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(CoreDX::C PROPERTIES IMPORTED_LOCATION_DEBUG "${CoreDX_C_LIBRARY_DEBUG}")
    endif ()
    set_target_properties(CoreDX::C PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CoreDX_INCLUDE_DIR}")

    if (CoreDX_FIND_REQUIRED_Ccf OR CoreDX_FIND_REQUIRED_Cxxcf)
        if (NOT TARGET CoreDX::Ccf)
            add_library(CoreDX::Ccf UNKNOWN IMPORTED)
        endif ()
        if (CoreDX_Ccf_LIBRARY_RELEASE)
            set_property(TARGET CoreDX::Ccf APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(CoreDX::Ccf PROPERTIES IMPORTED_LOCATION_RELEASE "${CoreDX_Ccf_LIBRARY_RELEASE}")
        endif ()
        if (CoreDX_Ccf_LIBRARY_DEBUG)
            set_property(TARGET CoreDX::Ccf APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(CoreDX::Ccf PROPERTIES IMPORTED_LOCATION_DEBUG "${CoreDX_Ccf_LIBRARY_DEBUG}")
        endif ()
        set_target_properties(CoreDX::Ccf PROPERTIES INTERFACE_LINK_LIBRARIES CoreDX::C)
    endif ()

    if (CoreDX_FIND_REQUIRED_Cdyntype OR CoreDX_FIND_REQUIRED_Cdyntypexml OR CoreDX_FIND_REQUIRED_Cxxdyntype)
        if (NOT TARGET CoreDX::Cdyntype)
            add_library(CoreDX::Cdyntype UNKNOWN IMPORTED)
        endif ()
        if (CoreDX_Cdyntype_LIBRARY_RELEASE)
            set_property(TARGET CoreDX::Cdyntype APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(CoreDX::Cdyntype PROPERTIES IMPORTED_LOCATION_RELEASE "${CoreDX_Cdyntype_LIBRARY_RELEASE}")
        endif ()
        if (CoreDX_Cdyntype_LIBRARY_DEBUG)
            set_property(TARGET CoreDX::Cdyntype APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(CoreDX::Cdyntype PROPERTIES IMPORTED_LOCATION_DEBUG "${CoreDX_Cdyntype_LIBRARY_DEBUG}")
        endif ()
        set_target_properties(CoreDX::Cdyntype PROPERTIES INTERFACE_LINK_LIBRARIES CoreDX::C)
    endif ()

    if (CoreDX_FIND_REQUIRED_Cdyntypexml)
        if (NOT TARGET CoreDX::Cdyntypexml)
            add_library(CoreDX::Cdyntypexml UNKNOWN IMPORTED)
        endif ()
        set_target_properties(CoreDX::Cdyntypexml PROPERTIES 
            IMPORTED_LOCATION "${CoreDX_Cdyntypexml_LIBRARY}"
            INTERFACE_LINK_LIBRARIES CoreDX::Cdyntype
        )
    endif ()

    if (CoreDX_FIND_REQUIRED_Cxx OR CoreDX_FIND_REQUIRED_Cxxcf OR CoreDX_FIND_REQUIRED_Cxxdyntype)
        if (NOT TARGET CoreDX::Cxx)
            add_library(CoreDX::Cxx UNKNOWN IMPORTED)
        endif ()
        if (CoreDX_Cxx_LIBRARY_RELEASE)
            set_property(TARGET CoreDX::Cxx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(CoreDX::Cxx PROPERTIES IMPORTED_LOCATION_RELEASE "${CoreDX_Cxx_LIBRARY_RELEASE}")
        endif ()
        if (CoreDX_Cxx_LIBRARY_DEBUG)
            set_property(TARGET CoreDX::Cxx APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(CoreDX::Cxx PROPERTIES IMPORTED_LOCATION_DEBUG "${CoreDX_Cxx_LIBRARY_DEBUG}")
        endif ()
        set_target_properties(CoreDX::Cxx PROPERTIES INTERFACE_LINK_LIBRARIES CoreDX::C)
    endif ()

    if (CoreDX_FIND_REQUIRED_Cxxcf)
        if (NOT TARGET CoreDX::Cxxcf)
            add_library(CoreDX::Cxxcf UNKNOWN IMPORTED)
        endif ()
        if (CoreDX_Cxxcf_LIBRARY_RELEASE)
            set_property(TARGET CoreDX::Cxxcf APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(CoreDX::Cxxcf PROPERTIES IMPORTED_LOCATION_RELEASE "${CoreDX_Cxxcf_LIBRARY_RELEASE}")
        endif ()
        if (CoreDX_Cxxcf_LIBRARY_DEBUG)
            set_property(TARGET CoreDX::Cxxcf APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(CoreDX::Cxxcf PROPERTIES IMPORTED_LOCATION_DEBUG "${CoreDX_Cxxcf_LIBRARY_DEBUG}")
        endif ()
        set_target_properties(CoreDX::Cxxcf PROPERTIES INTERFACE_LINK_LIBRARIES "CoreDX::Cxx;CoreDX::Ccf")
    endif ()

    if (CoreDX_FIND_REQUIRED_Cxxdyntype)
        if (NOT TARGET CoreDX::Cxxdyntype)
            add_library(CoreDX::Cxxdyntype UNKNOWN IMPORTED)
        endif ()
        if (CoreDX_Cxxdyntype_LIBRARY_RELEASE)
            set_property(TARGET CoreDX::Cxxdyntype APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(CoreDX::Cxxdyntype PROPERTIES IMPORTED_LOCATION_RELEASE "${CoreDX_Cxxdyntype_LIBRARY_RELEASE}")
        endif ()
        if (CoreDX_Cxxcf_LIBRARY_DEBUG)
            set_property(TARGET CoreDX::Cxxdyntype APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(CoreDX::Cxxdyntype PROPERTIES IMPORTED_LOCATION_DEBUG "${CoreDX_Cxxdyntype_LIBRARY_DEBUG}")
        endif ()
        set_target_properties(CoreDX::Cxxdyntype PROPERTIES INTERFACE_LINK_LIBRARIES "CoreDX::Cxx;CoreDX::Cdyntype")
    endif ()
endif ()

mark_as_advanced(
    CoreDX_INCLUDE_DIR
    CoreDX_C_LIBRARY_DEBUG
    CoreDX_C_LIBRARY_RELEASE
    CoreDX_Ccf_LIBRARY_DEBUG
    CoreDX_Ccf_LIBRARY_RELEASE
    CoreDX_Cdyntype_LIBRARY_DEBUG
    CoreDX_Cdyntype_LIBRARY_RELEASE
    CoreDX_Cdyntypexml_LIBRARY
    CoreDX_Cxx_LIBRARY_DEBUG
    CoreDX_Cxx_LIBRARY_RELEASE
    CoreDX_Cxxcf_LIBRARY_DEBUG
    CoreDX_Cxxcf_LIBRARY_RELEASE
    CoreDX_Cxxdyntype_LIBRARY_DEBUG
    CoreDX_Cxxdyntype_LIBRARY_RELEASE
)

# vim: set ts=4 sw=4 expandtab:

