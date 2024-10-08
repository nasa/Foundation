include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/FoundationTargets.cmake")
set(WITH_DDS "@WITH_DDS@" CACHE STRING "Selected DDS provider.")
if ("@CoreDX_ROOT@" MATCHES ".+")
    set(CoreDX_ROOT_DIR "@CoreDX_ROOT@" CACHE PATH "CoreDX root directory.")
endif ()
if ("@CoreDX_HOST@" MATCHES ".+")
    set(CoreDX_HOST "@CoreDX_HOST@" CACHE STRING "CoreDX host system identifier.")
endif ()
if ("@CoreDX_TARGET@" MATCHES ".+")
    set(CoreDX_TARGET "@CoreDX_TARGET@" CACHE STRING "CoreDX target system identifier.")
endif ()
if (WITH_DDS STREQUAL "CycloneDDS-CXX")
    find_dependency(iceoryx_binding_c REQUIRED)
endif ()
set(DDS_PROVIDER_COMP "@DDS_PROVIDER_COMP@" CACHE STRING "DDS provider-specific component.")
find_dependency(Threads REQUIRED)
if ("@DDS_PROVIDER_COMP@" MATCHES ".+")
    find_dependency(@WITH_DDS@ REQUIRED COMPONENTS @DDS_PROVIDER_COMP@)
else ()
    find_dependency(@WITH_DDS@ REQUIRED)
endif ()

