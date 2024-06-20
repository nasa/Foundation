# Tutorial Step 1: An Insistent "Hello World"

The very first step in this tutorial brings little useful function, but it is
excellent in showcasing the fundamental concepts that every Foundation-based
application developer must account for.

## Bootstrap the Project Tree

Starting from a blank directory would take some time to lay out and type.
Fortunately, the Foundation source tree brings with it a so-called "skeleton"
tree that can be copied to boostrap an application project. Giving the project
the name `TutorialStep1`, with the `FOUNDATION_ROOT` environment variable
pointing to the root folder of the Foundation tree:

```console
$ mkdir TutorialStep1
$ cd TutorialStep1
$ rsync -auv ${FOUNDATION_ROOT}/examples/SampleAppSkeleton/ .
```

Once copied over, the structure of the project tree should be as follows:

```
TutorialStep1
├─cmake
│ └─Modules
│   ├─CoreDXTargetIDLSources.cmake
│   ├─FindCoreDX.cmake
│   ├─FindOpenSplice.cmake
│   ├─OpenSpliceTargetIDLSources.cmake
│   └─TargetIDLSources.cmake
├─CMakeLists.txt
└─src
  ├─main.cpp
  ├─MyAppDelegate.cpp
  └─MyAppDelegate.h
```

Each file and folder serve a different purpose, as follows:

* `cmake/Modules` folder - Contains modules with convenience functions that can
  be called upon in the project's build configuration.
* `cmake/CoreDXTargetIDLSources.cmake` - Contains convenience functions used to
  identify and enumerate C++ files generated from an IDL file by the CoreDX IDL
  compiler.
* `cmake/Modules/FindCoreDX.cmake` - Contains convenience functions used to
  locate a CoreDX distribution and configure the project to use it.
* `cmake/Modules/FindOpenSplice.cmake` - Contains convenience functions used to
  locate an OpenSplice distribution and configure the project to use it.
* `cmake/OpenSpliceTargetIDLSources.cmake` - Contains convenience functions used
  to identify and enumerate C++ files generated from an IDL file by the
  OpenSplice IDL compiler.
* `cmake/TargetIDLSources.cmake` - Contains convenience functions that
  generalize the IDL-to-C++ source file identification and enumeration,
  leveraging `CoreDXTargetIDLSources.cmake` and
  `OpenSpliceTargetIDLSources.cmake` provided functions based on the selected
  DDS provider.
* `CMakeLists.txt` - Project build configuration entry point. The file defines
  the project build and install plan, first and foremost. In doing so, it also
  describes logic to resolve any project build dependencies.
* `src` folder - Contains all of the source code for the project.
* `src/main.cpp` file - Main entry point for the C++ application. The contents
  are minimal, as the majority of the application logic is elsewhere.
* `src/MyAppDelegate.{cpp,h}` - Class that serves as the starting point for all
  application-specific logic.

For brevity and conciseness, the tutorial will concentrate on just a few areas:

* The Project Build Configuration in `CMakeLists.txt`
* The application main function in `src/main.cpp`
* The application delegate class in `src/MyAppDelegate.{cpp,h}`

## Project Build Configuration

The `CMakeLists.txt` file describes how the project's build environment is
configured. The file defines the project, metadata, dependencies, and build
plan. A complete discussion on the structure of the file and its breadth of
functionality is beyond the scope of this tutorial, but may be found in the
[CMake Documentation][CMake_Documentation] web site. Of interest to this
discussion are the sections covered below.

### Project Metadata

Line 6 in the `CMakeLists.txt` file defines the metadata associated with the
project via the [`project()` command][CMake_project_cmd]. In this example this
includes:

* The project name (`SampleApp`)
* The programming languages used to implement its logic (`LANGUAGES CXX`)

In order to make the project unique, it should be given a unique name like
`InsistentHello`. Alter the contents of line 6 so that it reads as follows:

```cmake
project(InsistentHello LANGUAGES CXX)
```

There are other details that can be added, such as a version string, short
description, and URL address. For this example, however, they will not be
specified.

### Project Dependencies

Line 10 in `CMakeLists.txt` uses the
[`find_package()` command][CMake_find_package_cmd] to declare the Foundation as
a *required* dependency of our project. The statement calls out a very specific
dependency in the `CoreKit` module within Foundation.

### Target Definition

Line 12 in `CMakeLists.txt` uses the
[`add_executable()` command][CMake_add_executable_cmd] to define an executable
target the project will produce. Although there's no limit to the number of
targets that can be defined within `CMakeLists.txt`, this example will only
define one. The executable target receives the same name as given to the project
(see [Project Metadata](#project-metadata)) via the
[CMake `PROJECT_NAME` variable][CMake_PROJECT_NAME_var]. Immediately following
that is the list of source files, identified using paths relative to the project
root, that make up the executable target.

### Target Dependencies

Line 17 in `CMakeLists.txt` uses the
[`target_link_libraries()` CMake command][CMake_target_link_libraries_cmd] to
declare that the executable target depends on the `Foundation::CoreKit` module,
which was resolved in the
[section discussing the project dependencies](#project-dependencies). The
module notation somewhat mimics C++ in identifying the module using ownership
scope: `CoreKit` module within the `Foundation` declared dependency.

> [!NOTE]
> Developers that have used CMake lightly before may presume that the target
> dependency definition is incomplete, as this *seems* to only add library
> dependencies and not header file path dependencies. The argument provided to
> the `target_link_libraries()` command, however, is an
> [imported target][CMake_Import_Target]. Declaring the dependency in that
> single command brings in both library and header file configuration.

## Application Main Function

Just like any C++ executable, Foundation applications start off through a
`main()` function. Unlike typical executables, however, the content of this
function for all Foundation applications is very similar; there's rarely a need
to customize it outside of a few areas. Typically, those areas are:

* Creation of the application delegate instance (discussed at length in the
  [Application Delegate Class](#application-delegate-class) section).
* Configuration of the application name.

Line 17 of the `src/main.cpp` file shows the declaration of the variable meant
to hold the application delegate instance. Line 22 shows the creation of the
delegate instance, and line 29 shows creation of a `::CoreKit::Application`
instance that uses the newly-created delegate.

Line 37 shows the application overall initialization call. The first argument
provided to this call is the name that the application will identify with as it
produces log messages. The name should be changed to better match this example.
Alter the code so that it reads as follows:

```c++
G_MyApp->initialize("InsistentHello", argc, argv);
```

## Application Delegate Class

The Foundation, in and of itself, brings with it a lot of fundamental features
that are common to all applications. These features are all served from a
top-level class called [`::CoreKit::Application`](../../CoreKit/Application.h).
In order to accommodate all application-specific logic, instances of the
aforementioned class defer to (or "delegate") some tasks to an instance of a
class defined by developers. The class provided by developers must implement the
interface defined in the `::CoreKit::AppDelegate` class. In this example, this
delegate is defined in the `MyAppDelegate` class, via the `MyAppDelegate.h` and
`MyAppDelegate.cpp` source files.

The `::CoreKit::Application` class defers to the application-specific delegate
at specific times during the application's life cycle:

* Immediately after the application initializes.
* Just before the application is about to start reacting to stimuli.
* Just prior to application termination.
* Just after application termination.

### Application-Specific Logic

Some of the application delegate's custom logic is directly related to the
points at which `::CoreKit::Application` defers to it, but other logic is
independent of that. In this example application, the plan is to have a periodic
timer that emits a text message every time the timer expires. To have the
`MyAppDelegate` class handle timer expiration events, it must implement the
`::CoreKit::InterruptListener` interface.  Open the `src/MyAppDelegate.h` file
and alter the opening of the class definition in line 6 as follows:

```c++
class MyAppDelegate : public CoreKit::AppDelegate, public CoreKit::InterruptListener
```

The `::CoreKit::InterruptListener` interface defines methods that we must
override in order to receive timer expiration events. Add a new method
definition to the `MyAppDelegate` class just before its closing brace in line 52
with the following signature:

```c++
virtual void timerExpired(int timerId) override;
```

The `timerExpired()` method will be called by the application's run loop every
time a timer it manages expires. The `timerId` argument to the method is used
to identify the expiring timer. When a timer is registered with a run loop it
is assigned a unique identifier. Given that our example application only has
one timer, the identifier provided via `timerId` to `timerExpired()` is
superfluous, but not completely useless. There will be a need to use the timer
identifier in order to cancel it prior to application termination. Add the
following definitions to the `MyAppDelegate` class immediately after the
`timerExpired()` method definition:

```c++
private:
    int m_timerId;
```

The `m_timerId` instance field will be used to remember the assigned identifier
after the timer is registered. Now with internal state, it is good form to
initialize any and all state information to known values at construction time.
Open the `src/MyAppDelegate.cpp` file and alter the class constructor
implementation so it reads as follows:

```c++
MyAppDelegate::MyAppDelegate():
    m_timerId(-1)
{

}
```

All valid timer identifiers provided by the Foundation are greater than `0`, so
a value of `-1` in the `m_timerId` field is a clear indication that the timer
has not been configured.

The plan is to have the application emit a text message at every timer
expiration, which defines the implementation of the `timerExpired()` method.
Open the `src/MyAppDelegate.cpp` file and add this method implementation to the
bottom:

```c++
void
MyAppDelegate::timerExpired(int timerId)
{
    CoreKit::G_MyApp->log() << CoreKit::AppLog::LL_INFO << "Hello!" << CoreKit::EndLog;
}
```

The implementation makes use of another Foundation facility: logging. The
`CoreKit::G_MyApp` global variable is initialized in the `main()` function to
hold the `::CoreKit::Application` instance, and that class provides access to
the built-in logging facilities via the `log()` method. The method returns an
instance of the [`::CoreKit::AppLog` class](../../CoreKit/AppLog.h) class. That
class offers a C++ streams-like interface to produce log messages, where the
first value streamed in is the log severity, and subsequent values are used to
build the log message itself. The end of a log message must be indicated using
the `CoreKit::EndLog` sentinel constant. Suffice to say, that statement emits a
simple `Hello!` message with "information-level" severity. The message should
appear on the console used to run the application.

After all of the independent application logic is implemented, the next step is
to implement the logic that interfaces with `::CoreKit::Application`.

### Application Initialization

Immediately after the application performs basic initialization, the
`::CoreKit::Application` class defers to the developer's application delegate
instance via the `applicationDidInitialize()` method. When invoked, the
application delegate is given the opportunity to perform the following tasks
as necessary:

* Interpret any provided by the user via command-line arguments.
* Create instances of subordinate entities that will be used during the
  application's lifetime.

This example does not have either of those needs, so the method implementation
in `src/MyAppDelegate.cpp` can remain blank.

### Application Start

Just before surrendering control to the application's main run loop that
monitors stimuli, the `::CoreKit::Application` class defers to the application
delegate via the `applicationStarting()` method. When invoked, the application
delegate is given the opportunity to perform the following tasks as necessary:

* Activate any stimuli sources, both internal (such as timers or generic
  events) and external (such as serial ports or network sockets).

The application in this example does have an internal source of stimuli: the
periodic timer. Thus, the `applicationStarting()` implementation must configure
and start that timer. Alter the body of the `applicationStarting()` method so it
reads as follows:

```c++
void
MyAppDelegate::applicationStarting(Application *theApp)
{
    m_timerId = theApp->getMainRunLoop()->registerTimerWithInterval(1.0, this, true);
}
```

The `theApp` argument provided to `applicationStarting()` points to the
`::CoreKit::Application` instance created in the `main()` function. The
`::CoreKit::Application` class provides access to the application's main run
loop via its `getMainRunLoop()` method. The run loop is an instance of the
[`::CoreKit::RunLoop` class](../../CoreKit/RunLoop.h). That class provides timer
registration via its `registerTimerWithInterval()` method. The method accepts
three (3) arguments:

* The timer period expressed in seconds as a real number.
* A pointer to the instance implementing the `::CoreKit::InterruptListener`
  interface that will receive the timer expiration events.
* Whether the timer repeats (`true`) or is one-shot (`false`).

A more robust design would have the method logic enforce the pre-condition that
the timer be uninitialized upon entry (by checking it has a value of `-1`), but
for this simple example that will be omitted.

### Application Pre-Termination

Prior to the `::CoreKit::Application` class wresting control back from the main
run loop monitoring stimuli, it defers to the application delegate via the
`applicationWillTerminate()` method. When invoked, the application delegate is
given the opportunity to perform the following tasks as necessary:

* De-activate any stimuli sources configured at
  [Application Start](#application-start).

Again, the example application has one source of stimuli: the timer. Although
once this method is called the application is "not long for this world," it is
still good form to clean up as much as possible prior to termination. Alter the
bod of the `applicationWillTerminate()` method so it reads as follows:

```c++
void
MyAppDelegate::applicationWillTerminate(Application *theApp)
{
    if (m_timerId != -1)
    {
        theApp->getMainRunLoop()->deregisterTimer(m_timerId);
        m_timerId = -1;
    }
}
```

Just like with `applicationStarting()`, the `theApp` argument points to the
`::CoreKit::Application` instance for the application. In this implementation
there is a check to ensure that a timer is indeed active by checking the value
of the `m_timerId`. Once its confirmed a timer is active, it is deactivated via
the application main run loop using the `deregisterTimer()` method in
`::CoreKit::RunLoop`. After the timer is deactivated, the `m_timerId` field is
modified to reflect the state that the timer is no longer active.

### Application Post-Termination

Immediately after the main run loop is terminated, the `::CoreKit::Application`
class defers to the application delegate via the `applicationDidTerminate()`
method. When invoked, the application delegate is given the opportunity to
perform the following tasks as necessary:

* Release any resources (memory, file handles, etc.) held internally and by
  subordinate entities.

This example application does not allocate any subordinate or external
resources, so the method implementation in `src/MyAppDelegate.cpp` can remain
blank.

## Building

Once all of the application code is in place, the next step is to attempt a
build. It is useful to keep build artifacts separated from source code, so this
(and every other) example will use a separate directory tree to hold such build
products (typically referred to as a "build tree"). Create the build tree inside
the project root, and invoke the [CMake tool][CMake_Invocation] to configure it:

```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DFoundation_DIR=<Foundation Build Tree>/Foundation \
  -G Ninja ..
```

Where `<Foundation Build Tree>` refers to the directory that contains the build
tree for the Foundation, as created after following the
[Foundation build instructions](../building.md).

Once the build environment is configured, the next step is to actually run the
build:

```console
$ ninja
```

## Running

When the build is complete, the build tree should contain, among other items,
an executable called `InsistentHello`. Running this executable should yield
output similar to the following:

```console
$ ./InsistentHello 
[2024-05-21T21:36:32.955Z] [InsistentHello] [24091] [INFORMATION]: Hello!
[2024-05-21T21:36:33.955Z] [InsistentHello] [24091] [INFORMATION]: Hello!
[2024-05-21T21:36:34.955Z] [InsistentHello] [24091] [INFORMATION]: Hello!
[2024-05-21T21:36:35.955Z] [InsistentHello] [24091] [INFORMATION]: Hello!
...
```

The application can be stopped by issuing it a `SIGINT` signal via the keyboard
**Ctrl+C** combination. Graceful handling of the `SIGINT` signal is another
built-in feature of the Foundation. The Foundation also gracefully handles
receipt of a `SIGTERM` signal which is more common for applications that run in
the background.

## Conclusion

Congratulations! 🎉🎉 You have taken the very first and most important step in
learning how to write applications using the Foundation. The application itself
doesn't do much of anything, but it served as a great platform to showcase all
of the fundamentals that will come into play on future tutorials.


[CMake_Documentation]: https://cmake.org/documentation/ "CMake Documentation"
[CMake_Import_Target]: https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html "CMake Imported Targets"
[CMake_Invocation]: https://cmake.org/cmake/help/latest/manual/cmake.1.html "CMake Invocation"
[CMake_PROJECT_NAME_var]: https://cmake.org/cmake/help/latest/variable/PROJECT_NAME.html "CMake PROJECT_NAME variable"
[CMake_add_executable_cmd]: https://cmake.org/cmake/help/latest/command/add_executable.html "CMake add_executable() command"
[CMake_find_package_cmd]: https://cmake.org/cmake/help/latest/command/find_package.html "CMake find_package() command"
[CMake_project_cmd]: https://cmake.org/cmake/help/latest/command/project.html "CMake project() command"
[CMake_target_link_libraries_cmd]: https://cmake.org/cmake/help/latest/command/target_link_libraries.html "CMake target_link_libraries() command"
