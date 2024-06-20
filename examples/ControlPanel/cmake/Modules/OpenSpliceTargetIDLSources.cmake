# Licensed for U.S. Government use only.
#.rst:
# OpenSpliceTargetIDLSources
# --------------------------
#
# Add sources generated from IDL compilation to a project target (executable or
# library).
#
# The function prototype is as follows::
# 
#   opensplice_target_idl_sources(
#     <target_name>
#     [ BINDING <language> ]
#     [ INCLUDE_PATH_LIST <list_var> ]
#     [ OUTPUT_DIR <directory> ]
#     [ USE_EXISTING ]
#     IDL_SOURCES <source_file> ...
#   )
#
# Where the arguments carry the following meaning::
#
# * <target_name>: Attach the language binding generated source files to
#                  `<target_name>`.
# * BINDING <language>: Generate bindings for either C (`C`), C99 (`C99`), 
#                       C++ (`CXX`), ISO C++ (`ISOCPP`), or ISO C++ v2
#                       (`ISOCPP2`). Defaults to the target's `LINKER_LANGUAGE`
#                       property.
# * INCLUDE_PATH_LIST <list_var>: Name of CMake list variable that contains all
#                                 of the directoires to use as the include file
#                                 search path when compiling the IDL sources.
#                                 Defaults to the target's 
#                                 `INCLUDE_DIRECTORIES` property.
# * OUTPUT_DIR <directory>: Output language binding sources to the directory
#                           `<directory>`. If not specified, defaults to the
#                           current binary directory. If the specified
#                           directory is a relative path, it is assumed to be
#                           relative to the current binary directory.
# * USE_EXISTING: If found, the TARGET will be modified such that the
#                 appropriate language binding source files are added to it, but
#                 the language bindings will not be automatically generated
#                 during a build. Instead, the language binding source files
#                 will be expected to already be present. The special 
#                 '${TARGET}_binding' and 'clean_${TARGET}_binding' custom 
#                 targets will still be created so that language binding source
#                 file regeneration can still be done manually. Should only be
#                 used when the bindings are in the source tree, at the
#                 directory pointed to by 'OUTPUT_DIR'. Note that using this
#                 flag assumes that the already-existing language binding
#                 sources match the 'BINDING' specified (or inferred) for the
#                 IDL file(s).
# * IDL_SOURCES <source_file> ...: Names of the IDL source files to compile.
#                                  Any source files specified as a relative
#                                  path are assumed to be relative to the
#                                  current source directory.
#
function(OPENSPLICE_TARGET_IDL_SOURCES TargetName)
    cmake_parse_arguments(
        PARSE_ARGV 1
        "SPL_TIS"
        "USE_EXISTING"
        "BINDING;INCLUDE_PATH_LIST;OUTPUT_DIR"
        "IDL_SOURCES"
    )

    get_property(TARGET_DEFINED TARGET ${TargetName} PROPERTY NAME SET)

    if (NOT TARGET_DEFINED)
        message(FATAL_ERROR "opensplice_target_idl_sources(): Unknown target '${TargetName}'.")
    endif ()

    if (NOT SPL_TIS_IDL_SOURCES)
        message(FATAL_ERROR "opensplice_target_idl_sourceS(): No IDL sources identified.")
    endif ()

    foreach (AN_IDL_FILE IN LISTS SPL_TIS_IDL_SOURCES)
        if (NOT IS_ABSOLUTE ${AN_IDL_FILE})
            set(AN_IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${AN_IDL_FILE}")
        endif ()
        list(APPEND XformedIdlFiles "${AN_IDL_FILE}")
        if (NOT EXISTS ${AN_IDL_FILE})
            message(FATAL_ERROR "opensplice_target_idl_sources(): Cannot find IDL source file '${AN_IDL_FILE}'.")
        endif ()
    endforeach ()
    set(SPL_TIS_IDL_SOURCES ${XformedIdlFiles})

    if (NOT SPL_TIS_OUTPUT_DIR)
        set(SPL_TIS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif ()

    if (NOT SPL_TIS_BINDING)
        get_property(SPL_TIS_BINDING TARGET ${TargetName} LINKER_LANGUAGE)
    endif ()
    if (${SPL_TIS_BINDING} STREQUAL "C")
        set (SPL_TIS_BINDING "c")
    elseif (${SPL_TIS_BINDING} STREQUAL "C99")
        set (SPL_TIS_BINDING "c99")
    elseif (${SPL_TIS_BINDING} STREQUAL "CXX")
        set (SPL_TIS_BINDING "cpp")
    elseif (${SPL_TIS_BINDING} STREQUAL "ISOCPP")
        set (SPL_TIS_BINDING "isocpp")
    elseif (${SPL_TIS_BINDING} STREQUAL "ISOCPP2")
        set (SPL_TIS_BINDING "isocpp2")
    else ()
        message (FATAL_ERROR "opensplice_target_idl_sources(): Unrecognized binding '${SPL_TIS_BINDING}'.")
    endif ()

    if (NOT SPL_TIS_INCLUDE_PATH_LIST)
        get_property(SPL_TIS_INCLUDE_PATH_LIST TARGET ${TargetName} PROPERTY INCLUDE_DIRECTORIES)
    else ()
        set(SPL_TIS_INCLUDE_PATH_LIST ${${SPL_TIS_INCLUDE_PATH_LIST}})
    endif ()

    define_property(
        TARGET
        PROPERTY IDL_LANG_BINDING_HEADERS
        BRIEF_DOCS "List of header files produced from IDL file."
        FULL_DOCS "List of header files produced from IDL file."
    )

    # For each IDL source file, add the appropriate "custom command" that
    # produces the language binding source files, as well as add said source
    # files to the named target.
    foreach (IDL_FILE IN LISTS SPL_TIS_IDL_SOURCES)
        get_filename_component (IDL_BASENAME ${IDL_FILE} NAME_WE)
        if (SPL_TIS_BINDING STREQUAL "c")
            list(APPEND LANG_BINDING_HEADERS
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SacDcps.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.h"
            )
            list(APPEND LANG_BINDING_SOURCES
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SacDcps.c"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.c"
            )
            set (PpLang "C")
        elseif (SPL_TIS_BINDING STREQUAL "c99")
            list(APPEND LANG_BINDING_HEADERS
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SacDcps.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.h"
            )
            list(APPEND LANG_BINDING_SOURCES
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.c"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SacDcps.c"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.c"
            )
            set (PpLang "C99")
        elseif (SPL_TIS_BINDING STREQUAL "cpp")
            list(APPEND LANG_BINDING_HEADERS
                "${SPL_TIS_OUTPUT_DIR}/ccpp_${IDL_BASENAME}.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps_impl.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.h"
            )
            list(APPEND LANG_BINDING_SOURCES
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps_impl.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.cpp"
            )
            set (PpLang "C++")
        elseif (SPL_TIS_BINDING STREQUAL "isocpp")
            list(APPEND LANG_BINDING_HEADERS
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}_DCPS.hpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps_impl.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.h"
            )
            list(APPEND LANG_BINDING_SOURCES
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}Dcps_impl.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.cpp"
            )
            set (PpLang "ISO C++")
        elseif (SPL_TIS_BINDING STREQUAL "isocpp2")
            list(APPEND LANG_BINDING_HEADERS
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}_DCPS.hpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.h"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.h"
            )
            list(APPEND LANG_BINDING_SOURCES
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}.cpp"
                "${SPL_TIS_OUTPUT_DIR}/${IDL_BASENAME}SplDcps.cpp"
            )
            set (PpLang "ISO C++ v2")
        endif ()
        if (SPL_TIS_USE_EXISTING)
            add_custom_target(
                ${TargetName}_binding
                COMMAND
                    ${OpenSplice_IDLPP}
                    "-l" ${SPL_TIS_BINDING}
                    "-S"
                    "$<$<BOOL:${SPL_TIS_INCLUDE_PATH_LIST}>:-I$<JOIN:${SPL_TIS_INCLUDE_PATH_LIST},;-I>>"
                    "-d" "${SPL_TIS_OUTPUT_DIR}"
                    "${IDL_FILE}"
                COMMENT "Re-generating ${PpLang} source files from ${IDL_FILE}."
                COMMAND_EXPAND_LISTS
            )
            add_custom_target(
                clean_${TargetName}_binding
                "cmake"
                "-E"
                "remove"
                ${LANG_BINDING_HEADERS}
                ${LANG_BINDING_SOURCES}
                COMMAND_EXPAND_LISTS
                COMMENT "Cleaning out ${PpLang} source files generated from ${IDL_FILE}."
            )
        else ()
            add_custom_command(
                OUTPUT
                    ${LANG_BINDING_HEADERS}
                    ${LANG_BINDING_SOURCES}
                COMMAND ${OpenSplice_IDLPP} ARGS
                    "-l" ${SPL_TIS_BINDING}
                    "-S"
                    "$<$<BOOL:${SPL_TIS_INCLUDE_PATH_LIST}>:-I$<JOIN:${SPL_TIS_INCLUDE_PATH_LIST},;-I>>"
                    "-d" ${SPL_TIS_OUTPUT_DIR}
                    ${IDL_FILE}
                MAIN_DEPENDENCY ${IDL_FILE}
                COMMENT "Generating ${PpLang} source files from ${IDL_FILE}."
                COMMAND_EXPAND_LISTS
            )
            add_custom_target(
                ${TargetName}_binding
                DEPENDS
                    ${LANG_BINDING_HEADERS}
                    ${LANG_BINDING_SOURCES}
                COMMENT "Re-generating ${PpLang} source files from ${IDL_FILE}."
            )
            add_custom_target(
                clean_${TargetName}_binding
                "cmake"
                "-E"
                "remove"
                ${LANG_BINDING_HEADERS}
                ${LANG_BINDING_SOURCES}
                COMMAND_EXPAND_LISTS
            )
        endif ()
        set_property (
            TARGET ${TargetName} 
            APPEND PROPERTY SOURCES
                ${LANG_BINDING_HEADERS}
                ${LANG_BINDING_SOURCES}
        )
        set_property(
            TARGET ${TargetName}
            APPEND PROPERTY IDL_LANG_BINDING_HEADERS
            ${LANG_BINDING_HEADERS}
        )
    endforeach ()
endfunction(OPENSPLICE_TARGET_IDL_SOURCES)