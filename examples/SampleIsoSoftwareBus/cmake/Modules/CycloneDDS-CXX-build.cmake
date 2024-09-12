# =============================================================================
# CODE ATTRIBUTION NOTICE:
# The following functions are near identical to those offered by the CycloneDDS
# distribution. The code was modified in this project to better suit the target
# environment. Applicable licenses for this file below:
#
# Copyright(c) 2020 to 2022 ZettaScale Technology and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#

function(CYCLONE_CREATE_IDLC_TARGETS_GENERIC)
    set(options NO_TYPE_INFO USE_EXISTING)
    set(one_value_keywords TARGET BACKEND DEFAULT_EXTENSIBILITY BASE_DIR OUTPUT)
    set(multi_value_keywords FILES FEATURES INCLUDES WARNINGS SUFFIXES DEPENDS)
    cmake_parse_arguments(
        IDLC "${options}" "${one_value_keywords}" "${multi_value_keywords}" "" ${ARGN})

    # find idlc binary
    if(CMAKE_CROSSCOMPILING)
        find_program(_idlc_executable idlc NO_CMAKE_FIND_ROOT_PATH REQUIRED)

        if(_idlc_executable)
            set(_idlc_depends "")
        else()
            message(FATAL_ERROR "Cannot find idlc executable")
        endif()
    else()
        set(_idlc_executable CycloneDDS::idlc)
        set(_idlc_depends CycloneDDS::idlc)
    endif()

    if(NOT IDLC_TARGET AND NOT IDLC_FILES)
        # assume deprecated invocation: TARGET FILE [FILE..]
        list(GET IDLC_UNPARSED_ARGUMENTS 0 IDLC_TARGET)
        list(REMOVE_AT IDLC_UNPARSED_ARGUMENTS 0 IDLC_)
        set(IDLC_FILES ${IDLC_UNPARSED_ARGUMENTS})

        if(IDLC_TARGET AND IDLC_FILES)
            message(WARNING " Deprecated use of idlc_generate. \n"
                " Consider switching to keyword based invocation.")
        endif()

        # Java based compiler used to be case sensitive
        list(APPEND IDLC_FEATURES "case-sensitive")
    endif()

    if(NOT IDLC_TARGET)
        message(FATAL_ERROR "idlc_generate called without TARGET")
    elseif(NOT IDLC_FILES)
        message(FATAL_ERROR "idlc_generate called without FILES")
    endif()

    # remove duplicate features
    if(IDLC_FEATURES)
        list(REMOVE_DUPLICATES IDLC_FEATURES)
    endif()

    foreach(_feature ${IDLC_FEATURES})
        list(APPEND IDLC_ARGS "-f" ${_feature})
    endforeach()

    # add directories to include search list
    if(IDLC_INCLUDES)
        foreach(_dir ${IDLC_INCLUDES})
            list(APPEND IDLC_INCLUDE_DIRS "-I" ${_dir})
        endforeach()
    endif()

    # generate using which language (defaults to c)?
    if(IDLC_BACKEND)
        string(APPEND _language "-l" ${IDLC_BACKEND})
    endif()

    # set dependencies
    if(IDLC_DEPENDS)
        list(APPEND _depends ${_idlc_depends} ${IDLC_DEPENDS})
    else()
        set(_depends ${_idlc_depends})
    endif()

    if(IDLC_DEFAULT_EXTENSIBILITY)
        set(_default_extensibility ${IDLC_DEFAULT_EXTENSIBILITY})
        list(APPEND IDLC_ARGS "-x" ${_default_extensibility})
    endif()

    if(IDLC_WARNINGS)
        foreach(_warn ${IDLC_WARNINGS})
            list(APPEND IDLC_ARGS "-W${_warn}")
        endforeach()
    endif()

    if(IDLC_NO_TYPE_INFO)
        list(APPEND IDLC_ARGS "-t")
    endif()

    if(IDLC_BASE_DIR)
        file(REAL_PATH ${IDLC_BASE_DIR} _base_dir_abs)
        list(APPEND IDLC_ARGS "-b${_base_dir_abs}")
    endif()

    if(IDLC_OUTPUT)
        set(_dir ${IDLC_OUTPUT})
    else()
        set(_dir ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    list(APPEND IDLC_ARGS "-o${_dir}")
    set(_target ${IDLC_TARGET})

    foreach(_file ${IDLC_FILES})
        get_filename_component(_path ${_file} ABSOLUTE)
        list(APPEND _files "${_path}")
    endforeach()

    # set source suffixes (defaults to .c and .h)
    if(NOT IDLC_SUFFIXES)
        set(IDLC_SUFFIXES ".c" ".h")
    endif()

    set(_outputs "")
    set(_binding_gen_targets "")
    set(_binding_clean_targets "")

    foreach(_file ${_files})
        get_filename_component(_name ${_file} NAME_WE)
        get_filename_component(_name_ext ${_file} NAME)
        set (_binding_gen_target "${_name_ext}_binding")
        set (_binding_clean_target "clean_${_name_ext}_binding")
        list(APPEND _binding_gen_targets "${_binding_gen_target}")
        list(APPEND _binding_clean_targets "${_binding_clean_target}")

        # Determine middle path for directory reconstruction
        if(IDLC_BASE_DIR)
            file(RELATIVE_PATH _file_path_rel ${_base_dir_abs} ${_file})

            # Hard Fail
            if(_file_path_rel MATCHES "^\\.\\.")
                message(FATAL_ERROR "Cannot use base dir with different file tree from input file (${_base_dir_abs} to ${_file} yields ${_file_path_rel})")
            endif()

            string(REPLACE ${_name_ext} "" _mid_dir_path ${_file_path_rel})
            string(REGEX REPLACE "[\\/]$" "" _mid_dir_path ${_mid_dir_path})
        endif()

        set(_file_outputs "")

        foreach(_suffix ${IDLC_SUFFIXES})
            if(IDLC_BASE_DIR)
                list(APPEND _file_outputs "${_dir}/${_mid_dir_path}/${_name}${_suffix}")
            else()
                list(APPEND _file_outputs "${_dir}/${_name}${_suffix}")
            endif()
        endforeach()

        list(APPEND _outputs ${_file_outputs})
        # BUG WORKAROUND: The -o flag has no effect on the IDL compiler as of
        # this writing (2024-08-19). Instead the compiler always writes to the
        # directory where it was ran. Setting the current working directory of
        # the command via "WORKING_DIRECTORY" to the specified OUTPUT should
        # emulate the behavior of a properly-working "-o" flag. What's weird is
        # that the directory structure specified with the "-o" flag is created,
        # so the features only works somewhat.
        if (IDLC_USE_EXISTING)
            add_custom_target(
                "${_binding_gen_target}"
                COMMAND ${_idlc_executable} ${_language} ${IDLC_ARGS} ${IDLC_INCLUDE_DIRS} ${_file}
                WORKING_DIRECTORY "${_dir}"
                COMMENT "Re-generating C++ source files from ${_file}"
            )
            add_custom_target(
                "${_binding_clean_target}"
                COMMAND ${CMAKE_COMMAND} "-E" "remove" ${_file_outputs}
                COMMENT "Cleaning out C++ source files generated from ${_file}"
                COMMAND_EXPAND_LISTS
            )
            # Required for bootstrapping when the C++ files have not been generated
            foreach (_an_output ${_file_outputs})
                if (NOT EXISTS "${_an_output}")
                    file(WRITE "${_an_output}" "#error Re-generate C++ source files using ${_target}_binding target.\n")
                endif ()
            endforeach ()
        else ()
            add_custom_command(
                OUTPUT ${_file_outputs}
                COMMAND ${_idlc_executable}
                ARGS ${_language} ${IDLC_ARGS} ${IDLC_INCLUDE_DIRS} ${_file}
                DEPENDS ${_files} ${_depends}
                WORKING_DIRECTORY "${_dir}"
                COMMENT "Generating C++ source files for ${_file}."
            )
            add_custom_target(
                "${_binding_gen_target}"
                DEPENDS ${_file_outputs}
                COMMENT "Re-generating C++ source files from ${_file}."
            )
        endif ()
    endforeach()
    add_custom_target("${_target}_binding" DEPENDS ${_binding_gen_targets})
    if (IDLC_USE_EXISTING)
        add_custom_target("clean_${_target}_binding" DEPENDS ${_binding_clean_targets})
    endif ()
    add_library(${_target} STATIC ${_outputs})
endfunction()

function(CYCLONE_CREATE_IDLCXX_TARGETS)
    set(_OPTION_ARGS "USE_EXISTING")
    set(one_value_keywords TARGET DEFAULT_EXTENSIBILITY OUTPUT)
    set(multi_value_keywords FILES FEATURES INCLUDES WARNINGS)
    cmake_parse_arguments(
        IDLCXX "${_OPTION_ARGS}" "${one_value_keywords}" "${multi_value_keywords}" "" ${ARGN})

    # find idlcxx shared library
    if(CMAKE_CROSSCOMPILING)
        find_library(_idlcxx_shared_lib cycloneddsidlcxx NO_CMAKE_FIND_ROOT_PATH REQUIRED)

        if(_idlcxx_shared_lib)
            set(_idlcxx_depends "")
        else()
            message(FATAL_ERROR "Cannot find idlcxx shared library")
        endif()
    else()
        set(_idlcxx_shared_lib "$<TARGET_FILE:CycloneDDS-CXX::idlcxx>")
        set(_idlcxx_depends CycloneDDS-CXX::idlcxx)
    endif()
    if (IDLCXX_USE_EXISTING)
        set (_USE_EXISTING_ARG "USE_EXISTING")
    else ()
        set (_USE_EXISTING_ARG "")
    endif ()

    cyclone_create_idlc_targets_generic(TARGET ${IDLCXX_TARGET}
        BACKEND ${_idlcxx_shared_lib}
        FEATURES ${IDLCXX_FEATURES}
        INCLUDES ${IDLCXX_INCLUDES}
        WARNINGS ${IDLCXX_WARNINGS}
        DEFAULT_EXTENSIBILITY ${IDLCXX_DEFAULT_EXTENSIBILITY}
        SUFFIXES .hpp .cpp
        DEPENDS ${_idlcxx_depends}
        OUTPUT ${IDLCXX_OUTPUT}
        ${_USE_EXISTING_ARG}
        FILES ${IDLCXX_FILES}
    )

    if(CYCLONEDDS_CXX_ENABLE_LEGACY)
        target_include_directories(${IDLCXX_TARGET}
            PUBLIC ${Boost_INCLUDE_DIR}
        )
    endif()
endfunction()
# END CODE ATTRIBUTION NOTICE
# =============================================================================

macro (OUTSIDE_BUILD_TREE RESULT_VAR ABS_PATH_TO_CHECK)
    file(RELATIVE_PATH _REL_FROM_BUILD "${CMAKE_CURRENT_BINARY_DIR}" "${ABS_PATH_TO_CHECK}")
    if ("${_REL_FROM_BUILD}" MATCHES "^\\.\\.")
        set("${RESULT_VAR}" TRUE)
    else ()
        set("${RESULT_VAR}" FALSE)
    endif ()
endmacro ()
function (CREATE_SB_LIBRARY_TARGETS)
    set(_OPTION_ARGS "USE_EXISTING")
    set(_ONE_VALUE_ARGS "TARGET" "OUTPUT")
    set(_MULTI_VALUE_ARGS "IDL_FILES")
    cmake_parse_arguments(CSL "${_OPTION_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})
    set(_OUTPUT_ABS "${CMAKE_CURRENT_BINARY_DIR}")
    if (CSL_OUTPUT)
        set(_OUTPUT_ABS "${CSL_OUTPUT}")
        if (NOT IS_ABSOLUTE "${_OUTPUT_ABS}")
            get_filename_component(_OUTPUT_ABS "${_OUTPUT_ABS}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        endif ()
        OUTSIDE_BUILD_TREE(_OUTPUT_OUTSIDE_BUILD "${_OUTPUT_ABS}")
        if (_OUTPUT_OUTSIDE_BUILD AND NOT CSL_USE_EXISTING)
            message(WARNING "create_sb_library_targets(): Without USE_EXISTING, OUTPUT expected to be within build tree.")
        endif ()
        if (CSL_USE_EXISTING AND NOT _OUTPUT_OUTSIDE_BUILD)
            message(FATAL_ERROR "create_sb_library_targets(): Cannot USE_EXISTING with output within build tree.")
        endif ()
        unset (_OUTPUT_OUTSIDE_BUILD)
    elseif (CSL_USE_EXISTING)
        message(FATAL_ERROR "create_sb_library_targets(): Cannot USE_EXISTING with output within build tree.")
    endif ()
    if (CSL_USE_EXISTING)
        set (_USE_EXISTING_ARG "USE_EXISTING")
    else ()
        set (_USE_EXISTING_ARG "")
    endif ()
    cyclone_create_idlcxx_targets(TARGET ${CSL_TARGET} OUTPUT ${_OUTPUT_ABS} ${_USE_EXISTING_ARG} FILES ${CSL_IDL_FILES})
    target_include_directories(${CSL_TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${_OUTPUT_ABS}>
            $<INSTALL_INTERFACE:include/${CSL_TARGET}>
    )
    target_link_libraries(${CSL_TARGET} PUBLIC CycloneDDS-CXX::ddscxx)
    target_compile_features(${CSL_TARGET} PUBLIC cxx_std_17)
    get_property(_IDLC_SOURCES TARGET ${CSL_TARGET} PROPERTY SOURCES)
    foreach (_IDLC_GEN_FILE ${_IDLC_SOURCES})
        get_filename_component(_IDLC_GEN_FILE_EXT "${_IDLC_GEN_FILE}" LAST_EXT)
        if ("${_IDLC_GEN_FILE_EXT}" STREQUAL ".hpp")
            list(APPEND _IDLC_HEADERS "${_IDLC_GEN_FILE}")
        endif ()
    endforeach()
    set_target_properties(${CSL_TARGET}
        PROPERTIES
            IDL_LANG_BINDING_HEADERS ${_IDLC_HEADERS}
    )
endfunction ()

# vim: set ts=4 sw=4 expandtab:
