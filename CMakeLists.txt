# ============================================================================
# Build Configuration for the Foundation SDK
# ============================================================================

cmake_minimum_required(VERSION 3.16)
project(Foundation
    VERSION 1.1.0
    DESCRIPTION "Event driven framework for Linux-based applications."
    LANGUAGES CXX
)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")
include(CheckSymbolExists)
include(CheckIncludeFileCXX)
find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_THREAD_LIBS_INIT)
        list (APPEND PUBLIC_DEFINES "_REENTRANT")
    endif ()
    list (APPEND PUBLIC_DEFINES "_GNU_SOURCE")
endif ()

set(WITH_DDS "OpenSplice" CACHE STRING "DDS provider to use")
set(WITH_DDSKIT "Classic" CACHE STRING "DdsKit flavor to build (\"Classic\" or \"ISO\")")

# =============================================================================
# Common OS SDK configuration
# =============================================================================

check_symbol_exists(SFD_NONBLOCK sys/signalfd.h SFD_NONBLOCK_FOUND)
check_symbol_exists(TFD_NONBLOCK sys/timerfd.h TFD_NONBLOCK_FOUND)
check_symbol_exists(EFD_NONBLOCK sys/eventfd.h EFD_NONBLOCK_FOUND)
check_symbol_exists(timerfd_create sys/timerfd.h TIMERFD_CREATE_FOUND)
check_symbol_exists(eventfd sys/eventfd.h EVENTFD_FOUND)
check_symbol_exists(signalfd sys/signalfd.h SIGNALFD_FOUND)
check_symbol_exists (pipe2 unistd.h PIPE2_FOUND)
check_symbol_exists(PF_CAN sys/socket.h PF_CAN_FOUND)
check_symbol_exists(SOCK_NONBLOCK sys/socket.h SOCK_NONBLOCK_FOUND)

if (NOT TIMERFD_CREATE_FOUND)
    message (FATAL_ERROR "timerfd_create() function required.")
endif ()

if (NOT EVENTFD_FOUND)
    message (FATAL_ERROR "eventfd() function required.")
endif ()

if (NOT SIGNALFD_FOUND)
    message (FATAL_ERROR "signalfd() function required.")
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    check_include_file_cxx (syslog.h SYSLOG_H_LOC)
    if (SYSLOG_H_LOC)
        list (APPEND OS_SDK_DEFINES "-DHAVE_SYSLOG_H=1")
    endif ()
    check_symbol_exists (syslog syslog.h SYSLOG_FOUND)
    if (SYSLOG_FOUND)
        list (APPEND OS_SDK_DEFINES "-DHAVE_SYSLOG=1")
    endif ()
    if (SFD_NONBLOCK_FOUND)
        list (APPEND OS_SDK_DEFINES "-DHAVE_SFD_NONBLOCK=1")
    endif ()
    if (TFD_NONBLOCK_FOUND)
        list (APPEND OS_SDK_DEFINES "-DHAVE_TFD_NONBLOCK=1")
    endif ()
    if (EFD_NONBLOCK_FOUND)
        list (APPEND OS_SDK_DEFINES "-DHAVE_EFD_NONBLOCK=1")
    endif ()
endif ()

# =============================================================================
# CoreKit build plan
# =============================================================================

set (CoreKitSources
        "CoreKit/AppDelegate.cpp"
        "CoreKit/AppDelegate.h"
        "CoreKit/Application.cpp"
        "CoreKit/Application.h"
        "CoreKit/AppLog.cpp"
        "CoreKit/AppLog.h"
        "CoreKit/BlockGuard.cpp"
        "CoreKit/BlockGuard.h"
        "CoreKit/BoundMember.h"
        "CoreKit/ByteVector.h"
        "CoreKit/CmdLineMultiArg.cpp"
        "CoreKit/CmdLineMultiArg.h"
        "CoreKit/CoreKit.h"
        "CoreKit/EventInputSource.cpp"
        "CoreKit/EventInputSource.h"
        "CoreKit/factory.h"
        "CoreKit/FixedAllocator.h"
        "CoreKit/InputSource.cpp"
        "CoreKit/InputSource.h"
        "CoreKit/InterruptListener.cpp"
        "CoreKit/InterruptListener.h"
        "CoreKit/InvalidInputException.cpp"
        "CoreKit/InvalidInputException.h"
        "CoreKit/OsErrorException.cpp"
        "CoreKit/OsErrorException.h"
        "CoreKit/PreconditionNotMetException.cpp"
        "CoreKit/PreconditionNotMetException.h"
        "CoreKit/prodinfo.h"
        "CoreKit/RunLoop.cpp"
        "CoreKit/RunLoop.h"
        "CoreKit/RuntimeErrorException.cpp"
        "CoreKit/RuntimeErrorException.h"
        "CoreKit/SignalInputSource.cpp"
        "CoreKit/SignalInputSource.h"
        "CoreKit/StaticAllocator.h"
        "CoreKit/SynchronizedRunLoop.cpp"
        "CoreKit/SynchronizedRunLoop.h"
        "CoreKit/SystemTime.cpp"
        "CoreKit/SystemTime.h"
        "CoreKit/Thread.cpp"
        "CoreKit/Thread.h"
        "CoreKit/ThreadDelegate.cpp"
        "CoreKit/ThreadDelegate.h"
        "CoreKit/TimerInputSource.cpp"
        "CoreKit/TimerInputSource.h"
        "CoreKit/WatchdogExpiredCallback.cpp"
        "CoreKit/WatchdogExpiredCallback.h"
        "CoreKit/WatchdogExpiredCallbackT.h"
        "CoreKit/WatchdogTimer.cpp"
        "CoreKit/WatchdogTimer.h"
    )
add_library(
    CoreKit
    SHARED
        ${CoreKitSources}
)

target_compile_definitions(
    CoreKit
    PUBLIC ${PUBLIC_DEFINES}
    PRIVATE ${OS_SDK_DEFINES}
)

target_include_directories(
    CoreKit
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/CoreKit"
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    CoreKit
    PUBLIC
        Threads::Threads
)

target_compile_features(
    CoreKit
    PUBLIC
        cxx_std_17
)

set_target_properties(
    CoreKit
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
)

# =============================================================================
# CanBusKit build plan
# =============================================================================

add_library(
    CanBusKit
    SHARED
        "CanBusKit/CanBusFrameCallback.cpp"
        "CanBusKit/CanBusFrameCallback.h"
        "CanBusKit/CanBusFrameCallbackT.h"
        "CanBusKit/CanBusFrameCallbackWithPredT.h"
        "CanBusKit/CanBusFrameNotification.cpp"
        "CanBusKit/CanBusFrameNotification.h"
        "CanBusKit/CanBusIo.cpp"
        "CanBusKit/CanBusIo.h"
        "CanBusKit/CanBusKit.h"
)

message (STATUS "PF_CAN_FOUND=${PF_CAN_FOUND}")
target_compile_definitions(
    CanBusKit
    PRIVATE
    $<$<BOOL:{PF_CAN_FOUND}>:PF_CAN=29>
    $<$<BOOL:{SOCK_NONBLOCK_FOUND}>:HAVE_SOCK_NONBLOCK=1>
    PUBLIC
        ${PUBLIC_DEFINES}
)

target_include_directories(
    CanBusKit
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/CanBusKit"
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    CanBusKit
    PUBLIC
    CoreKit
)

target_compile_features(
    CanBusKit
    PUBLIC
        cxx_std_17
)

set_target_properties(
    CanBusKit
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
)

# =============================================================================
# Classic C++ DdsKit build plan
# =============================================================================

if (WITH_DDSKIT STREQUAL "Classic")
    if (WITH_DDS STREQUAL "CoreDX")
        set (DDS_PROVIDER_COMP "Cxxcf")
        set (DDS_PROVIDER_LIB "CoreDX::Cxxcf")
        set (DDS_PROVIDER "DDS_PROVIDER_COREDX")
        set (dds_frag_file_name "DdsKit/coredx_includes.h.frag")
    elseif (WITH_DDS STREQUAL "OpenSplice")
        set (DDS_PROVIDER_COMP "Sacpp")
        set (DDS_PROVIDER_LIB "OpenSplice::Sacpp")
        set (DDS_PROVIDER "DDS_PROVIDER_OSPL")
        set (dds_frag_file_name "DdsKit/opensplice_includes.h.frag")
    else ()
        message (FATAL_ERROR "Missing or unknown DDS provider '${WITH_DDS}'.")
    endif ()
    find_package(${WITH_DDS} REQUIRED COMPONENTS ${DDS_PROVIDER_COMP})

    add_library(
        DdsKit
        SHARED
            "DdsKit/BaseDdsTopic.cpp"
            "DdsKit/BaseDdsTopic.h"
            "DdsKit/BasicDdsParticipant.cpp"
            "DdsKit/BasicDdsParticipant.h"
            "DdsKit/ConfiguredDdsTopic.h"
            "DdsKit/dds_provider_ids.h"
            "DdsKit/DdsErrorException.cpp"
            "DdsKit/DdsErrorException.h"
            "DdsKit/DdsKit.h"
            "DdsKit/PeerMonitoring.cpp"
            "DdsKit/PeerMonitoring.h"
            "DdsKit/seq_utils.h"
            "DdsKit/string_utils.cpp"
            "DdsKit/string_utils.h"
            "DdsKit/TargetedSampleCallback.h"
            "DdsKit/TargetedSampleCallbackWithPredT.h"
            "DdsKit/ThroughputCap.cpp"
            "DdsKit/ThroughputCap.h"
    )

    file(READ "${dds_frag_file_name}" dds_provider_includes)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/DdsKit/dds_provider.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/DdsKit/dds_provider.h"
    )

    message (STATUS "PIPE2_FOUND=${PIPE2_FOUND}")
    target_compile_definitions(
        DdsKit
        PRIVATE
            $<$<BOOL:PIPE2_FOUND>:HAVE_PIPE2=1>
            ${OS_SDK_DEFINES}
        PUBLIC
            ${PUBLIC_DEFINES}
    )

    target_include_directories(
        DdsKit
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/DdsKit"
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
            $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(
        DdsKit
        PUBLIC
            CoreKit
            ${DDS_PROVIDER_LIB}
    )

    target_compile_features(
        DdsKit PUBLIC cxx_std_17
    )

    set_target_properties(
        DdsKit
        PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION 0
    )
endif (WITH_DDSKIT STREQUAL "Classic")

# =============================================================================
# ISO C++ DdsKit build plan
# =============================================================================

if (WITH_DDSKIT STREQUAL "ISO")
    if (WITH_DDS STREQUAL "CycloneDDS-CXX")
        unset (DDS_PROVIDER_COMP)
        set (DDS_PROVIDER_LIB "CycloneDDS-CXX::ddscxx" "iceoryx_binding_c::iceoryx_binding_c")
        find_package(iceoryx_binding_c REQUIRED)
        find_package(${WITH_DDS} REQUIRED)
    elseif (WITH_DDS STREQUAL "CoreDX")
        set (DDS_PROVIDER_COMP "IsoCxx")
        set (DDS_PROVIDER_LIB "CoreDX::IsoCxx")
        find_package(${WITH_DDS} REQUIRED COMPONENTS ${DDS_PROVIDER_COMP})
    else ()
        message (FATAL_ERROR "Missing or unknown DDS provider '${WITH_DDS}'.")
    endif ()

    add_library(
        IsoDdsKit
        SHARED
            "IsoDdsKit/BasicRandomIdSource.cpp"
            "IsoDdsKit/BasicRandomIdSource.hpp"
            "IsoDdsKit/DdsEnabledAppDelegate.cpp"
            "IsoDdsKit/DdsEnabledAppDelegate.hpp"
            "IsoDdsKit/DeferredActions.cpp"
            "IsoDdsKit/DeferredActions.hpp"
            "IsoDdsKit/EmitNewReaderDataAction.hpp"
            "IsoDdsKit/EmitStatusAction.hpp"
            "IsoDdsKit/IsoDdsKit.h"
            "IsoDdsKit/ReaderContext.hpp"
            "IsoDdsKit/ReaderStimuliMux.hpp"
            "IsoDdsKit/TopicCollection.cpp"
            "IsoDdsKit/TopicCollection.hpp"
            "IsoDdsKit/UniqueIdSource.hpp"
            "IsoDdsKit/ValueDistribution.hpp"
            "IsoDdsKit/WriterContext.hpp"
            "IsoDdsKit/WriterStimuliMux.hpp"
            "IsoDdsKit/status_types.hpp"
    )
    target_compile_definitions(
        IsoDdsKit
        PRIVATE
            $<$<BOOL:PIPE2_FOUND>:HAVE_PIPE2=1>
            ${OS_SDK_DEFINES}
        PUBLIC
            ${PUBLIC_DEFINES}
    )

    target_include_directories(
        IsoDdsKit
        PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/IsoDdsKit"
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
            $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(
        IsoDdsKit
        PUBLIC
            CoreKit
            ${DDS_PROVIDER_LIB}
    )

    target_compile_features(
        IsoDdsKit PUBLIC cxx_std_17
    )

    set_target_properties(
        IsoDdsKit
        PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION 0
    )
endif (WITH_DDSKIT STREQUAL "ISO")

# =============================================================================
# NetworkKit build plan
# =============================================================================

add_library(
    NetworkKit
    SHARED
        "NetworkKit/ConnectionCallback.cpp"
        "NetworkKit/ConnectionCallback.h"
        "NetworkKit/ConnectionCallbackT.h"
        "NetworkKit/ConnectionNotification.cpp"
        "NetworkKit/ConnectionNotification.h"
        "NetworkKit/ConnectionStates.h"
        "NetworkKit/NetworkKit.h"
        "NetworkKit/TcpClient.cpp"
        "NetworkKit/TcpClient.h"
        "NetworkKit/TcpMessageCallback.cpp"
        "NetworkKit/TcpMessageCallback.h"
        "NetworkKit/TcpMessageCallbackT.h"
        "NetworkKit/TcpMessageInputSource.cpp"
        "NetworkKit/TcpMessageInputSource.h"
        "NetworkKit/TcpMessageNotification.cpp"
        "NetworkKit/TcpMessageNotification.h"
        "NetworkKit/TcpServerInputSource.cpp"
        "NetworkKit/TcpServerInputSource.h"
        "NetworkKit/TcpSocket.cpp"
        "NetworkKit/TcpSocket.h"
        "NetworkKit/UdpPacketDistribution.cpp"
        "NetworkKit/UdpPacketDistribution.h"
        "NetworkKit/UdpPacketDistribution.hh"
        "NetworkKit/UdpPacketNotification.cpp"
        "NetworkKit/UdpPacketNotification.h"
        "NetworkKit/UdpPacketNotification.hh"
        "NetworkKit/UdpSocket.cpp"
        "NetworkKit/UdpSocket.h"
        "NetworkKit/UdpSocket.hh"
)

target_compile_definitions(
    NetworkKit
    PRIVATE ${OS_SDK_DEFINES}
    PUBLIC ${PUBLIC_DEFINES}
)

target_include_directories(
    NetworkKit
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/NetworkKit"
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(NetworkKit PUBLIC CoreKit)

target_compile_features(
    NetworkKit
    PUBLIC
        cxx_std_17
)

set_target_properties(
    NetworkKit
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
)

# =============================================================================
# SerialKit build plan
# =============================================================================

add_library(
    SerialKit
    SHARED
        "SerialKit/SerialDataCallback.cpp"
        "SerialKit/SerialDataCallback.h"
        "SerialKit/SerialDataCallbackT.h"
        "SerialKit/SerialDataNotification.cpp"
        "SerialKit/SerialDataNotification.h"
        "SerialKit/SerialIo.cpp"
        "SerialKit/SerialIo.h"
        "SerialKit/SerialKit.h"
        "SerialKit/SerialUtilities.h"
        "SerialKit/SerialUtilities.cpp"
)

target_compile_definitions(
    SerialKit
    PRIVATE
        ${OS_SDK_DEFINES}
    PUBLIC
        ${PUBLIC_DEFINES}
)

target_include_directories(
    SerialKit
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/SerialKit"
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    SerialKit
    PUBLIC
    CoreKit
)

target_compile_features(
    SerialKit
    PUBLIC
        cxx_std_17
)

set_target_properties(
    SerialKit
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 0
)

# =============================================================================
# Foundation library installation plan
# =============================================================================

list(APPEND INSTALL_TARGETS CoreKit NetworkKit SerialKit CanBusKit)
if (WITH_DDSKIT STREQUAL "Classic")
    list(APPEND INSTALL_TARGETS DdsKit)
elseif (WITH_DDSKIT STREQUAL "ISO")
    list(APPEND INSTALL_TARGETS IsoDdsKit)
endif ()

install(
    TARGETS ${INSTALL_TARGETS}
    EXPORT
        FoundationTargets
    COMPONENT
        Bin
    LIBRARY DESTINATION
        "lib"
    ARCHIVE DESTINATION
        "lib"
    RUNTIME DESTINATION
        "bin"
)

install(
    FILES
        "CoreKit/AppDelegate.h"
        "CoreKit/Application.h"
        "CoreKit/AppLog.h"
        "CoreKit/BlockGuard.h"
        "CoreKit/BoundMember.h"
        "CoreKit/ByteVector.h"
        "CoreKit/CmdLineMultiArg.h"
        "CoreKit/CoreKit.h"
        "CoreKit/EventInputSource.h"
        "CoreKit/factory.h"
        "CoreKit/FixedAllocator.h"
        "CoreKit/InputSource.h"
        "CoreKit/InterruptListener.h"
        "CoreKit/InvalidInputException.h"
        "CoreKit/OsErrorException.h"
        "CoreKit/PreconditionNotMetException.h"
        "CoreKit/prodinfo.h"
        "CoreKit/RunLoop.h"
        "CoreKit/RuntimeErrorException.h"
        "CoreKit/SignalInputSource.h"
        "CoreKit/StaticAllocator.h"
        "CoreKit/SynchronizedRunLoop.h"
        "CoreKit/SystemTime.h"
        "CoreKit/Thread.h"
        "CoreKit/ThreadDelegate.h"
        "CoreKit/TimerInputSource.h"
        "CoreKit/WatchdogExpiredCallback.h"
        "CoreKit/WatchdogExpiredCallbackT.h"
        "CoreKit/WatchdogTimer.h"
    DESTINATION
        "include/CoreKit"
    COMPONENT
        Dev
)

install(
    FILES
        "NetworkKit/ConnectionCallback.h"
        "NetworkKit/ConnectionCallbackT.h"
        "NetworkKit/ConnectionNotification.h"
        "NetworkKit/ConnectionStates.h"
        "NetworkKit/NetworkKit.h"
        "NetworkKit/TcpClient.h"
        "NetworkKit/TcpMessageCallback.h"
        "NetworkKit/TcpMessageCallbackT.h"
        "NetworkKit/TcpMessageInputSource.h"
        "NetworkKit/TcpMessageNotification.h"
        "NetworkKit/TcpServerInputSource.h"
        "NetworkKit/TcpSocket.h"
    DESTINATION
        "include/NetworkKit"
    COMPONENT
        Dev
)

install(
    FILES
        "SerialKit/SerialDataCallback.h"
        "SerialKit/SerialDataCallbackT.h"
        "SerialKit/SerialDataNotification.h"
        "SerialKit/SerialIo.h"
        "SerialKit/SerialKit.h"
    DESTINATION
        "include/SerialKit"
    COMPONENT
        Dev
)

install(
    FILES
        "CanBusKit/CanBusFrameCallback.h"
        "CanBusKit/CanBusFrameCallbackT.h"
        "CanBusKit/CanBusFrameCallbackWithPredT.h"
        "CanBusKit/CanBusFrameNotification.h"
        "CanBusKit/CanBusIo.h"
        "CanBusKit/CanBusKit.h"
    DESTINATION
        "include/CanBusKit"
    COMPONENT
        Dev
)


if (WITH_DDSKIT STREQUAL "Classic")
    install(
        FILES
            "DdsKit/BaseDdsTopic.h"
            "DdsKit/BasicDdsParticipant.h"
            "DdsKit/ConfiguredDdsTopic.h"
            "DdsKit/dds_provider_ids.h"
            "DdsKit/DdsErrorException.h"
            "DdsKit/DdsKit.h"
            "DdsKit/PeerMonitoring.h"
            "DdsKit/seq_utils.h"
            "DdsKit/string_utils.h"
            "DdsKit/TargetedSampleCallback.h"
            "DdsKit/TargetedSampleCallbackWithPredT.h"
            "DdsKit/ThroughputCap.h"
            "${CMAKE_CURRENT_BINARY_DIR}/DdsKit/dds_provider.h"
        DESTINATION
            "include/DdsKit"
        COMPONENT
            Dev
    )
elseif (WITH_DDSKIT STREQUAL "ISO")
    get_target_property(ISO_DDS_KIT_SOURCES IsoDdsKit SOURCES)
    foreach (AN_ISO_DDS_KIT_SOURCE ${ISO_DDS_KIT_SOURCES})
        get_filename_component(ISO_DDS_KIT_SOURCE_EXT "${AN_ISO_DDS_KIT_SOURCE}" LAST_EXT)
        if ((${ISO_DDS_KIT_SOURCE_EXT} STREQUAL ".hpp") OR (${ISO_DDS_KIT_SOURCE_EXT} STREQUAL ".h"))
            list (APPEND ISO_DDS_KIT_HEADERS "${AN_ISO_DDS_KIT_SOURCE}")
        endif ()
    endforeach ()
    install(
        FILES
            ${ISO_DDS_KIT_HEADERS}
        DESTINATION
            "include/IsoDdsKit"
        COMPONENT
            Dev
    )
endif ()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Foundation/FoundationConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

export(EXPORT FoundationTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/Foundation/FoundationTargets.cmake"
    NAMESPACE Foundation::
)
configure_file(
    "cmake_local/Foundation/FoundationConfig.cmake"
    "Foundation/FoundationConfig.cmake"
    @ONLY
)

set(ConfigPackageLocation "lib/cmake/Foundation")
install(EXPORT FoundationTargets
    FILE
        FoundationTargets.cmake
    NAMESPACE
        Foundation::
    DESTINATION
        "${ConfigPackageLocation}"
    COMPONENT
        Dev
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/Foundation/FoundationConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/Foundation/FoundationConfigVersion.cmake"
    DESTINATION
        "${ConfigPackageLocation}"
    COMPONENT
        Dev
)

# =============================================================================
# Packaging plan for Foundation
# =============================================================================

set(CPACK_PACKAGE_VENDOR "UBE-KSC-NASA")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${WITH_DDS}-${WITH_DDSKIT}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_CONTACT "Rolando J. Nieves <rolando.j.nieves@nasa.gov>")
if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CPACK_GENERATOR "DEB")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "TGZ")
endif ()
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENT_Bin_REQUIRED ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CPACK_DEBIAN_PACKAGE_NAME "${PROJECT_NAME}-${WITH_DDS}-${WITH_DDSKIT}")
set(CPACK_DEBIAN_PACKAGE_RELEASE "dev")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")

if ("${CMAKE_CURRENT_BINARY_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    include(CPack)
    include(CPackComponent)
endif ()

# =============================================================================
# Compile command information pseudo-target
# =============================================================================

if (UNIX AND CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        compile_commands ALL
        COMMAND
            "${CMAKE_COMMAND}"
            "-E"
            "create_symlink"
            "${PROJECT_BINARY_DIR}/compile_commands.json"
            "${PROJECT_SOURCE_DIR}/compile_commands.json"
        DEPENDS
            "${PROJECT_BINARY_DIR}/compile_commands.json"
    )
endif ()

# vim: set ts=4 sw=4 expandtab:

