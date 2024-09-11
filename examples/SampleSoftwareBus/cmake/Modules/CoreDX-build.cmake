function (COREDX_CREATE_DDL_TARGETS)
    set (_OPTION_ARGS "USE_EXISTING")
    set (_ONE_VALUE_ARGS "TARGET" "OUTPUT")
    set (_MULTI_VALUE_ARGS "IDL_FILES" "INCLUDES")
    cmake_parse_arguments(CCDT "${_OPTION_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})
    set(_output_file_suffixes ".cc" ".hh" "TypeSupport.cc" "TypeSupport.hh")
    set (_all_file_outputs "")
    foreach (_idl_file IN LISTS CCDT_IDL_FILES)
        if (NOT IS_ABSOLUTE "${_idl_file}")
            set(_idl_file "${CMAKE_CURRENT_SOURCE_DIR}/${_idl_file}")
        endif ()
        get_filename_component(_name ${_idl_file} NAME_WE)
        get_filename_component(_name_ext ${_idl_file} NAME)
        set (_binding_gen_target "${_name_ext}_binding")
        set (_binding_clean_target "clean_${_name_ext}_binding")
        list(APPEND _binding_gen_targets "${_binding_gen_target}")
        list(APPEND _binding_clean_targets "${_binding_clean_target}")
        set(_idl_file_outputs "")
        foreach (_suffix IN LISTS _output_file_suffixes)
            list(APPEND _idl_file_outputs "${CCDT_OUTPUT}/${_name}${_suffix}")
        endforeach ()
        list(APPEND _all_file_outputs ${_idl_file_outputs})
        set(_coredx_ddl_args "-l" "cpp11" "-d" "${CCDT_OUTPUT}")
        foreach (_include_dir IN LISTS CCDT_INCLUDES)
            list(APPEND _coredx_ddl_args "-I${_include_dir}")
        endforeach ()
        if (CCDT_USE_EXISTING)
            add_custom_target(
                "${_binding_gen_target}"
                COMMAND CoreDX::DdlCompiler ${_coredx_ddl_args} ${_idl_file}
                COMMENT "Re-generating C++ source files from ${_idl_file}"
            )
            add_custom_target(
                "${_binding_clean_target}"
                COMMAND ${CMAKE_COMMAND} "-E" "remove" ${_idl_file_outputs}
                COMMENT "Cleaning out C++ source files generated from ${_idl_file}"
                COMMAND_EXPAND_LISTS
            )
            # Required for bootstrapping when the C++ files have not been generated
            foreach (_an_output IN LISTS _idl_file_outputs)
                if (NOT EXISTS "${_an_output}")
                    file(WRITE "${_an_output}" "#error Re-generate C++ source files using ${CCDT_TARGET}_binding target.\n")
                endif ()
            endforeach ()
        else ()
            add_custom_command(
                OUTPUT ${_idl_file_outputs}
                COMMAND CoreDX::DdlCompiler 
                ARGS ${_coredx_ddl_args} ${_idl_file}
                DEPENDS ${_idl_file}
                COMMENT "Generating C++ source files for ${_idl_file}."
            )
            add_custom_target(
                "${_binding_gen_target}"
                DEPENDS ${_idl_file_outputs}
                COMMENT "Re-generating C++ source files from ${_idl_file}."
            )
        endif ()
    endforeach ()
    add_custom_target("${CCDT_TARGET}_binding" DEPENDS ${_binding_gen_targets})
    if (IDLC_USE_EXISTING)
        add_custom_target("clean_${CCDT_TARGET}_binding" DEPENDS ${_binding_clean_targets})
    endif ()
    add_library(${CCDT_TARGET} STATIC ${_all_file_outputs})
endfunction ()

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
    coredx_create_ddl_targets(TARGET ${CSL_TARGET} OUTPUT ${_OUTPUT_ABS} ${_USE_EXISTING_ARG} IDL_FILES ${CSL_IDL_FILES})
    target_include_directories(${CSL_TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${_OUTPUT_ABS}>
            $<INSTALL_INTERFACE:include/${CSL_TARGET}>
    )
    target_link_libraries(${CSL_TARGET} PUBLIC CoreDX::IsoCxx)
    get_property(_IDLC_SOURCES TARGET ${CSL_TARGET} PROPERTY SOURCES)
    foreach (_IDLC_GEN_FILE IN LISTS _IDLC_SOURCES)
        get_filename_component(_IDLC_GEN_FILE_EXT "${_IDLC_GEN_FILE}" LAST_EXT)
        if ("${_IDLC_GEN_FILE_EXT}" STREQUAL ".hh")
            list(APPEND _IDLC_HEADERS "${_IDLC_GEN_FILE}")
        endif ()
    endforeach()
    set_property(TARGET ${CSL_TARGET} PROPERTY IDL_LANG_BINDING_HEADERS ${_IDLC_HEADERS})
endfunction ()

# vim: set ts=4 sw=4 expandtab:
