# Licensed for U.S. Government use only.
#.rst:
# CoreDXTargetIDLSources
# ----------------------
#
# Add sources generated from IDL compilation to a project target (executable or
# library).
#
# The function prototype is as follows::
# 
#   coredx_target_idl_sources(
#     <target_name>
#     [ BINDING <language> ]
#     [ ENDIANNESS <endianness> ]
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
# * BINDING <language>: Generate bindings for either C (`C`) or C++ (`CXX`).
#                       Defaults to the target's `LINKER_LANGUAGE` property.
# * ENDIANNESS <endianness>: Generate bindings for the specific endianness, 
#                            such as little (`LITTLE`) or big (`BIG`).
#                            Defaults to the perceived endianness as 
#                            interpreted from the `CMAKE_SYSTEM_PROCESSOR`
#                            variable. Currently only implemented to detect
#                            Intel, ARM, or PowerPC.
# * INCLUDE_PATH_LIST <list_var>: Name of CMake list variable that contains all
#                                 of the directoires to use as the include file
#                                 search path when compiling the IDL sources.
#                                 Defaults to the target's 
#                                 `INCLUDE_DIRECTORIES` property.
# * OUTPUT_DIR <directory>: Output language binding sources to the directory
#                           `<directory>`. If not specified, defaults to the
#                           current binary directory. An output directory 
#                           specified as a relative path is assumed to be
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
#                 sources match the 'BINDING' and 'ENDIANNESS' specified (or
#                 inferred) for the IDL file(s).
# * IDL_SOURCES <source_file> ...: Names of the IDL source files to compile.
#                                  Any source files specified as a relative
#                                  path are assumed to be relative to the
#                                  current source directory.
#

include(GuessEndianness)

function(COREDX_TARGET_IDL_SOURCES TargetName)
    cmake_parse_arguments(
        PARSE_ARGV 1
        "CDX_TIS"
        "USE_EXISTING"
        "BINDING;ENDIANNESS;INCLUDE_PATH_LIST;OUTPUT_DIR"
        "IDL_SOURCES"
    )

    get_property(TARGET_DEFINED TARGET ${TargetName} PROPERTY NAME SET)

    if (NOT TARGET_DEFINED)
        message(FATAL_ERROR "coredx_target_idl_sources(): Unknown target '${TargetName}'.")
    endif ()

    if (NOT CDX_TIS_IDL_SOURCES)
        message(FATAL_ERROR "coredx_target_idl_sourceS(): No IDL sources identified.")
    endif ()

    foreach (AN_IDL_FILE IN LISTS CDX_TIS_IDL_SOURCES)
        if (NOT IS_ABSOLUTE ${AN_IDL_FILE})
            set(AN_IDL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${AN_IDL_FILE}")
        endif ()
        list(APPEND XformedIdlFiles "${AN_IDL_FILE}")
        if (NOT EXISTS ${AN_IDL_FILE})
            message(FATAL_ERROR "coredx_target_idl_sources(): Cannot find IDL source file '${AN_IDL_FILE}'.")
        endif ()
    endforeach ()
    set(CDX_TIS_IDL_SOURCES ${XformedIdlFiles})

    if (NOT CDX_TIS_OUTPUT_DIR)
        set(CDX_TIS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif ()

    if (NOT CDX_TIS_BINDING)
        get_property(CDX_TIS_BINDING TARGET ${TargetName} PROPERTY LINKER_LANGUAGE)
    endif ()
    if (${CDX_TIS_BINDING} STREQUAL "C")
        set (CDX_TIS_BINDING "c")
    elseif (${CDX_TIS_BINDING} STREQUAL "CXX")
        set (CDX_TIS_BINDING "cpp")
    else ()
        message (FATAL_ERROR "coredx_target_idl_sources(): Unrecognized binding '${CDX_TIS_BINDING}'.")
    endif ()

    if(NOT CDX_TIS_ENDIANNESS)
        guess_endianness(CDX_TIS_ENDIANNESS)
        mark_as_advanced(CDX_TIS_ENDIANNESS)
    endif ()
    if(CDX_TIS_ENDIANNESS STREQUAL "LITTLE")
        set (CDX_TIS_ENDIANNESS "-el")
    elseif(CDX_TIS_ENDIANNESS STREQUAL "BIG")
        set (CDX_TIS_ENDIANNESS "-eb")
    endif()

    if (NOT CDX_TIS_INCLUDE_PATH_LIST)
        get_property(CDX_TIS_INCLUDE_PATH_LIST TARGET ${TargetName} PROPERTY INCLUDE_DIRECTORIES)
    else ()
        set(CDX_TIS_INCLUDE_PATH_LIST ${${CDX_TIS_INCLUDE_PATH_LIST}})
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
    foreach (IDL_FILE IN LISTS CDX_TIS_IDL_SOURCES)
        get_filename_component (IDL_BASENAME ${IDL_FILE} NAME_WE)
        if (CDX_TIS_BINDING STREQUAL "c")
            set (SrcSuffix "c")
            set (HdrSuffix "h")
            set (PpLang "C")
        elseif (CDX_TIS_BINDING STREQUAL "cpp")
            set (SrcSuffix "cc")
            set (HdrSuffix "hh")
            set (PpLang "C++")
        endif ()
        list (APPEND LANG_BINDING_HEADERS
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}.${HdrSuffix}"
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}DataReader.${HdrSuffix}"
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}DataWriter.${HdrSuffix}"
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}TypeSupport.${HdrSuffix}"
        )
        list (APPEND LANG_BINDING_SOURCES
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}.${SrcSuffix}"
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}DataReader.${SrcSuffix}"
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}DataWriter.${SrcSuffix}"
            "${CDX_TIS_OUTPUT_DIR}/${IDL_BASENAME}TypeSupport.${SrcSuffix}"
        )
        if (CDX_TIS_USE_EXISTING)
            add_custom_target(
                ${TargetName}_binding
                    ${CoreDX_DDLC}
                    "-f" "${IDL_FILE}"
                    "-l" "${CDX_TIS_BINDING}"
                    "-d" "${CDX_TIS_OUTPUT_DIR}"
                    "${CDX_TIS_ENDIANNESS}"
                    "$<$<BOOL:${CDX_TIS_INCLUDE_PATH_LIST}>:-I$<JOIN:${CDX_TIS_INCLUDE_PATH_LIST},;-I>>"
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
                COMMAND ${CoreDX_DDLC} ARGS
                    "-f" ${IDL_FILE}
                    "-l" ${CDX_TIS_BINDING}
                    "-d" ${CDX_TIS_OUTPUT_DIR}
                    ${CDX_TIS_ENDIANNESS}
                    "$<$<BOOL:${CDX_TIS_INCLUDE_PATH_LIST}>:-I$<JOIN:${CDX_TIS_INCLUDE_PATH_LIST},;-I>>"
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
endfunction(COREDX_TARGET_IDL_SOURCES)

# vim: set ts=4 sw=4 expandtab:
 
