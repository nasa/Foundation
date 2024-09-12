# Building and Using the Foundation SDK

## Pre-Requisites

The Foundation SDK is a set of application libraries implemented using C++ and
targeting GNU/Linux operating systems. As such, building the SDK from source
bears these generic pre-requisites:

* A GNU/Linux distribution using Linux kernel 4.0 or greater.
* A C++ compiler that, at a minimum, supports the C++11 standard.
* The [CMake] build system, version 3.12 or newer.
* A CMake-compatible build tool.
* A compatible Data Distribution Service (DDS) package.

As of this writing (2024-05-20), the following specific packages are actively
tested against the Foundation SDK:

* For GNU/Linux:
  * Ubuntu Linux version 20.04 and 22.04, running on Intel x86-64
  * Embedded Linux as produced by Yocto version 2.4.4 or greater, running on
    an ARMv7 CPU.
* For the C++ compiler:
  * LLVM CLang version 10 or newer (preferred)
  * GCC version 8 or newer
* For the build tool:
  * [Ninja] build tool version 1.12.1 or newer (preferred)
  * GNU Make
* For the DDS distribution (when using `DdsKit`):
  * [CoreDX] version 5 or newer from Twin Oaks Computing, Inc.
  * [OpenSplice Community Edition][OpenSplice] version 6.9 or newer from ADLINK
* For the DDS distribution (when using `IsoDdsKit`):
  * [CoreDX] version 5.19 or newer, built against GCC 5 or newer, from Twin Oaks Computing, Inc.
  * [Cyclone DDS][CycloneDDS] version 0.10.5 or newer, built with shared memory ([iceoryx]) and C++ support.

> [!NOTE]
> All examples in the tutorial assume the following build/runtime
> pre-requisites:
>
> * OS: Ubuntu Linux on Intel x86-64
> * C++ compiler: LLVM Clang
> * Build tool: Ninja
> * Using `DdsKit` (via the `WITH_DDSKIT=Classic` setting)
> * DDS distribution: [OpenSplice]

### DDS Pre-Requisites: CoreDX

When building the Foundation with the [CoreDX DDS][CoreDX] libraries, the build
environment must be prepared as follows beforehand:

* CoreDX target and host files must be installed on the host. Typically, Twin
  Oaks Computing distributes both target and host files in a single archive.
* The `COREDX_TOP` environment variable must contain the fully-qualified
  file system location of the CoreDX installation.
* The `COREDX_HOST` environment variable must contain the CoreDX-assigned label
  of the host platform doing the build. As of this writing (2024-05-20), for
  CoreDX 5.6.0 on Linux x86-64, the platform label is `Linux_2.6_x86_64_gcc43`.
  The assigned label can be read off the contents of the `$COREDX_TOP/host`
  directory.
* The `COREDX_TARGET` environment variable must contain the CoreDX-assigned
  label of the targeted platform. In the example documented in the
  [Building](#building) section, this happens to be the same as the host
  platform. The assigned label can be read off the contents of the
  `$COREDX_TOP/target` directory.
* The `TWINOAKS_LICENSE_FILE` environment variable must point to the license
  file provided by Twin Oaks Computing after purchasing a developer and run time
  license.
* The CoreDX libraries must be present in the dynamic linker's run time
  bindings. This is usually accomplished by adding an entry to the bindings
  input at `/etc/ld.so.conf` pointing at the CoreDX libraries directory and
  running the `ldconfig` tool with elevated privileges.

### DDS Pre-Requisites: OpenSplice

When building the Foundation with the [OpenSplice] libraries, the build
environment must be prepared as follows beforehand:

* Follow the [OpenSplice build instructions][OpenSplice_Linux_Build] to produce
  the required "Stand-Alone C++" (SACPP) variant of the distribution.
* The OpenSplice distributions usually deploy an environment setup script in
  its `HDE` (Host Development Environment) sub-folder called `release.com`.
  Sourcing this script in the shell environment that will be used to run the
  steps in the [Building](#building) section should be sufficient.

> [!NOTE]
> [OpenSplice] is only supported when the Foundation is configured to build
> with the `DdsKit` component.

### DDS Pre-Requisites: Cyclone DDS

When building the Foundation with the [Cyclone DDS][CycloneDDS] libraries, the
build environment need not go through preparatory steps. The only requirement
is that, when building the Foundation library, the install locations to both
[iceoryx] and [Cyclone DDS][CycloneDDS] be specified using the CMake
`CMAKE_PREFIX_PATH` variable. For example, assume the packages are installed
as follows:

| Package | Location |
| :------ | :------- |
| **iceoryx** | `/opt/iceoryx/2.95.0` |
| **Cyclone DDS** (both C and C++) | `/opt/cyclonedds/0.10.5` |

With those locations, populating the `CMAKE_PREFIX_PATH` variable with the
value `/opt/iceoryx/2.95.0\;/opt/cyclonedds/0.10.5` (making sure the semicolon
is escaped) will point the Foundation build plan in the right direction.

> [!NOTE]
> [Cyclone DDS][CycloneDDS] is only supported when the Foundation is configured
> to build with the `IsoDdsKit` component.

## Building

Prior to configuring the Foundation build, users must select what build tool and
DDS distributions to use. The Foundation can be cross-compiled for an embedded
target, but for the sake of simplicity these build steps assume the target
platform is the build host. The steps assume using the [OpenSplice] DDS
distribution, the [Ninja] build tool, the "classic" `DdsKit`, and the CLang
compiler. Starting from the root folder of the Foundation checkout:

1. Create a build tree (omit if one already exists):
   ```console
   $ mkdir build
   ```
1. Make the build tree the current working directory:
   ```console
   $ cd build
   ```
1. Configure the build tree
   ```console
   $ cmake -DCMAKE_BUILD_TYPE=Debug \
     -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
     -DCMAKE_CXX_COMPILER=clang++ \
     -DWITH_DDSKIT=Classic \
     -DWITH_DDS=OpenSplice \
     -G Ninja \
     ..
   ```
1. Build the Foundation libraries
   ```console
   $ ninja
   ```

After the build is complete, the build tree may be used by Foundation-based
applications during the regular write-compile-test cycles. When the need to
distribute the build products arises, there are several options available to
package the Foundation for deployment. All depend on a build tree that contains
the completed build products.

> [!NOTE]
> Use of [CoreDX] requires a commercial license. The example build configuration
> command references [OpenSplice] in order to enhance developer accessibility to
> the Foundation SDK. If a commercial license for [CoreDX] is available, it can
> be used with just a change to the `-DWITH_DDS=` configuration line.

### Building with IsoDdsKit

The instructions shown in the [Building](#building) section mostly apply as-is
when building Foundation with support for the
[OMG ISO/IEC C++ PSM][OmgIsoCxxPsm] in the `IsoDdsKit` component. The only
differences manifest in the step used to configure the build tree:

1. Replace the `-DWITH_DDSKIT=Classic` command-line parameter with
  `-DWITH_DDSKIT=ISO`
1. Replace the `-DWITH_DDS=` command-line parameter with a reference to a
  compatible DDS distribution:
    1. `-DWITH_DDS=CycloneDDS-CXX` for [Cyclone DDS][CycloneDDS].
    1. `-DWITH_DDS=CoreDX` for [CoreDX].
1. Add any required build configuration parameters required by the DDS
  distribution ([Cyclone DDS][CycloneDDS] may need them; see
  [DDS Pre-Requisites: Cyclone DDS](#dds-pre-requisites-cyclone-dds)).

### Packaging: TAR/GZ Archives

The most basic method of packaing the Foundation for distribution is via TAR/GZ
archives. On target systems without a package manager this would be the only
viable option. After a build is complete, from the completed build tree:

1. Use the [CMake] CPack tool to create the distributable archives:
   ```console
   $ cpack -G TGZ
   ```

The `cpack` tool will leave two (2) TAR/GZ archives in the build tree bearing
a name conforming to the following structure:

```
Foundation-<version>-<DDS>-<target>-(Bin|Dev).tar.gz
```

Where:
* `<version>` - The major.minor.patch version number for the Foundation.
* `<DDS>` - The DDS distribution used to build (`OpenSplice` in the previous
  example).
* `<target>` - A label describing the target platform.
* `Bin` - Binary libraries needed both at runtime and during development.
* `Dev` - Header and other supporting files only needed during development.


### Packaging: DEB Archives

A method of distribution available to Debian-based targets is DEB packages.
After a build is complete, from the completed build tree:

1. Use the [CMake] CPack tool to create the packages:
   ```console
   $ cpack -G DEB
   ```

The `cpack` tool will leave two(2) DEB packages in the build tree bearing a
name conforming to the following structure:

> [!NOTE]
> An immediate differece to TAR/GZ archives is the exclusive use of lowercase
> characters, as recommended by the Debian packaging standards.

```
foundation-<DDS>-(bin|dev)_<version>-dev_<target>.deb
```

Where:
* `<DDS>` - The DDS distribution used to build (`opensplice` in the previous
  example).
* `bin` - Binary libraries needed both at runtime and during development.
* `dev` - Header and other supporting files only needed during development.
* `<version>` - The major.minor.patch version number for the Foundation.
* `<target>` - A label describing the target platform.

## Using

As stated in the [Building](#building) section, a build tree with built products
may be used as-is for regular development. Assuming the application project that
uses Foundation also relies on the [CMake] build system (highly recommended),
the application's `CMakeLists.txt` file must declare the explicit dependency
to Foundation with the following statement:

```cmake
find_package(Foundation REQUIRED)
```

In addition, the application target(s) making use of Foundation must add the
required import targets as dependencies. Assuming an executable target called
`MyApp`:

```cmake
target_link_libraries(
    MyApp
    Foundation::CoreKit
    Foundation::CanBusKit
    Foundation::NetworkKit
    Foundation::SerialKit
    Foundation::DdsKit
)
```

The set of import targets that must be added to the `target_link_libraries()`
depends on what features the particular target requires of the Foundation. What
each import target brings is as follows:

* `Foundation::CoreKit` - Core library for Foundation; *all* Foundation-based
  applications must include this import target.
* `Foundation::CanBusKit` - Library required by applications using Foundation's
  CAN bus features.
* `Foundation::NetworkKit` - Library required by applications using Foundation's
  TCP or UDP features.
* `Foundation::SerialKit` - Library required by applications using Foundation's
  serial port features.
* `Foundation::DdsKit` - Library required by applications using Foundation's
  DDS features.


Once the project's `CMakeLists.txt` file is updated accordingly, the build tree
must be configured with the following extra flag:

```
-DFoundation_DIR=<Foundation build tree>/Foundation
```

Where `<Foundation build tree>` is the directory path to the Foundation build
tree. The `Foundation` sub-directory under the Foundation build tree contains
configuration files that let [CMake] find all the pertinent header and library
locations.

> [!TIP]
> Several of the tutorial examples refer to a `FOUNDATION_DIR` environment
> variable that points to the root folder of the Foundation project. It is quite
> handy to have such an environment definition available when going through the
> [Tutorial](./tutorial.md).


[CMake]: https://cmake.org/ "CMake Build System"
[CoreDX]: https://www.twinoakscomputing.com/coredx "CoreDX DDS"
[CycloneDDS]: https://cyclonedds.io/ "Cyclone DDS"
[Ninja]: https://ninja-build.org/ "Ninja Build System"
[OmgIsoCxxPsm]: https://www.omg.org/spec/DDS-PSM-Cxx/1.0/About-DDS-PSM-Cxx "OMG ISO/IEC C++ PSM"
[OpenSplice]: https://github.com/ADLINK-IST/opensplice "Vortex OpenSplice Community Edition"
[OpenSplice_Linux_Build]: https://github.com/ADLINK-IST/opensplice#posix--linux "OpenSplice POSIX/Linux Build Instructions"
[iceoryx]: https://cyclonedds.io/ "iceoryx Shared Memory IPC"