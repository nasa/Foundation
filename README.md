# Tutorial

A tutorial on how to use the Foundation SDK is
[available here](./docs/tutorial.md).

# Introduction and a Quick Tour

The Foundation library contains a class library that system software developers
rely on to facilitate implementation of event-driven applications using C++. It
is made up of four (4) components that specialize on different areas.

## CoreKit
Foundation's CoreKit component is a class and function library that provides 
the bare basics that all system software applications, embedded or otherwise,
can benefit from.

### Basic Application Infrastructure
Among the bare basics provided by Foundation's CoreKit are the classes that
define and implement basic application functionality and features. This area of
CoreKit includes things like:

- Application structure
- Command line argument management
- Message logging
- System time utilities

### Run Loop
Another important feature provided by Foundation's CoreKit is the event
multiplexing run loop that manages all application stimuli, be they internally
or externally generated. Based on configuration offered by the application,
the run loop dispatches portions of the application logic in response to these
stimuli.

### Stimuli Sources
The stimuli that interact with an application's run loop span from the generic,
application specified kind to other more common stimulus sources such as:

- Timers (periodic and one shot)
- Operating System Signals
- Operating System I/O

### Concurrent Logic
For the most part, applications built using the Foundation are structured to
run as single-threaded, event driven applications. The application's single
thread, via the run loop, manages all incoming stimuli giving the false 
impression of concurrent operation. When such behavior is insufficient, and
an application requires actual concurrent behavior, Foundation's CoreKit
provides features to accommodate, such as:

- Concurrent thread management
- Synchronization objects
- Thread local run loops

## DdsKit
Foundation's DdsKit component simplifies writing applications that are required
to interface with a Data Distribution System (DDS) based communications bus. 
The library offers near turn-key implementations for the most common tasks
required of DDS-enabled applications.

### DDS Participant
The primary requirement of any application that wishes to participate in a 
DDS-based communication bus (otherwise known as a DDS Domain) is the creation
of a domain participant, along with several other entities. DdsKit's class
library includes a basic DDS participant implementation that is configured
with the appropriate entities to satisfy the needs of a vast majority of
DDS-enabled applications. In addition, DdsKit integrates all DDS-borne stimuli
onto CoreKit's run loop.

### DDS Topics and IDL Integration
A DDS domain exchanges information by having developers design messages
via C-like language known as the Interface Definition Language (IDL) and
cataloguing these messages into groups known as Topics. DdsKit, making a set of
educated assumptions based on observed use patterns, simplifies this
message exchange mechanism for application developers. The inherent flexibility
provided by basic DDS is limited somewhat, but the simplicity dividend to 
application developers more than makes up for this lost flexibility.

## NetworkKit
Foundation's NetworkKit component provides tried and true implementations for
the most common needs for applications required to interact with a network
directly either via TCP/IP or UDP/IP.

### TCP/IP Networking
For applications that require direct access to TCP/IP, NetworkKit provides 
ready to use class libraries that simplify the creation of TCP/IP servers
and clients. NetworkKit also integrates with CoreKit's run loop any stimuli 
generated from these TCP/IP servers and clients, such as:

- Listening for incoming client connections
- Initiating a connection to a server
- Accepting incoming client connections
- Handling incoming data

### UDP/IP Networking
Applications requiring direct interaction with connection-less UDP/IP based
services, NetworkKit similarly provides class libraries that simplify 
management of these communication end points, while also integrating all 
associated stimuli with CoreKit's run loop.

## CanBusKit
Foundation's CanBusKit component brings with it the ability to integrate CAN
Bus interaction into a system software application, via CoreKit's run loop.
The component relies on the Linux-based [SocketCAN] libraries which come 
pre-installed on most modern Linux distributions. Hardware adapters connected
to the host platform must also have device drivers compatible with SocketCAN.

## SerialKit
Foundation's SerialKit component simplifies integrating RS-232, RS-422, and
RS-485 serial I/O into an embedded software application. SerialKit provides
simplified serial port configuration facilities, as well as integration of
serial port related stimuli into CoreKit's run loop. SerialKit depends on
Linux's [TermIO] subsystem, an intrinsic part of the OS. Any serial devices
to be used with SerialKit must be compatible with the Linux TermIO subsystem.


[SocketCAN]: https://www.kernel.org/doc/html/next/networking/can.html "Linux SocketCAN"
[TermIO]: https://man7.org/linux/man-pages/man3/termios.3.html "Linux TermIO"
