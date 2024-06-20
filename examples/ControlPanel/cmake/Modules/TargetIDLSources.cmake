function(TARGET_IDL_SOURCES TargetName)
    cmake_parse_arguments(
        PARSE_ARGV 1
        TIS
        "USE_EXISTING"
        "DDS_PROVIDER;BINDING;ENDIANNESS;INCLUDE_PATH_LIST;OUTPUT_DIR"
        "IDL_SOURCES"
    )

    if (NOT TIS_DDS_PROVIDER)
        message(FATAL_ERROR "No DDS provider identified.")
    endif ()

    if (NOT TIS_IDL_SOURCES)
        message(FATAL_ERROR "No IDL sources identified.")
    endif ()

    if (TIS_USE_EXISTING)
        set (USE_EXISTING_KW "USE_EXISTING")
    else ()
        set (USE_EXISTING_KW "")
    endif ()

    if (TIS_BINDING)
        set (BINDING_KW "BINDING")
    else ()
        set(BINDING_KW "")
    endif ()

    if (TIS_ENDIANNESS)
        set (ENDIANNESS_KW "ENDIANNESS")
    else ()
        set (ENDIANNESS_KW "")
    endif ()

    if (TIS_OUTPUT_DIR)
        set (OUTPUT_DIR_KW "OUTPUT_DIR")
    else ()
        set (OUTPUT_DIR_KW "")
    endif ()

    if (TIS_INCLUDE_PATH_LIST)
        set (INCLUDE_PATH_LIST_KW "INCLUDE_PATH_LIST")
    else ()
        set (INCLUDE_PATH_LIST_KW "")
    endif ()

    include(${TIS_DDS_PROVIDER}TargetIDLSources)

    if (${TIS_DDS_PROVIDER} STREQUAL "CoreDX")
        coredx_target_idl_sources(
            ${TargetName}
            ${BINDING_KW} ${TIS_BINDING}
            ${ENDIANNESS_KW} ${TIS_ENDIANNESS}
            ${USE_EXISTING_KW}
            ${OUTPUT_DIR_KW} ${TIS_OUTPUT_DIR}
            ${INCLUDE_PATH_LIST_KW} ${TIS_INCLUDE_PATH_LIST}
            IDL_SOURCES ${TIS_IDL_SOURCES}
        )
        set(DDS_HEADER_EXT "hh" PARENT_SCOPE)
    elseif (${TIS_DDS_PROVIDER} STREQUAL "OpenSplice")
        opensplice_target_idl_sources(
            ${TargetName}
            ${BINDING_KW} ${TIS_BINDING}
            ${USE_EXISTING_KW}
            ${OUTPUT_DIR_KW} ${TIS_OUTPUT_DIR}
            ${INCLUDE_PATH_LIST_KW} ${TIS_INCLUDE_PATH_LIST}
            IDL_SOURCES ${TIS_IDL_SOURCES}
        )
        set(DDS_HEADER_EXT "h" PARENT_SCOPE)
    elseif (${TIS_DDS_PROVIDER} STREQUAL "OpenDDS")
        opendds_target_idl_sources(
            ${TargetName}
            ${USE_EXISTING_KW}
            ${OUTPUT_DIR_KW} ${TIS_OUTPUT_DIR}
            ${INCLUDE_PATH_LIST_KW} ${TIS_INCLUDE_PATH_LIST}
            IDL_SOURCES ${TIS_IDL_SOURCES}
        )
        set(DDS_HEADER_HEXT "h" PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Unknown DDS provider '${TIS_DDS_PROVIDER}'.")
    endif ()
endfunction(TARGET_IDL_SOURCES)
