## Building

Configure the makefile by invoking `cmake` in the build directory.
Do not invoke `cmake` in any component's root directory or any other path.

     # -DCMAKE_BUILD_TYPE=Debug - attach symbols to the output
     # -Dgroup_output=ON        - group library output in build/lib, executables
     #                            in build/bin, and test executables in build/test.
     # -Dglobal_output_path=ON  - output to ../../build instead of ./
     # -Dbuild_tests=OFF        - don't build test executables

     cd core/build/
     cmake -DCMAKE_BUILD_TYPE=Debug ../
     make
     ./smacore_test   # or make test

Project sources are declared in `CMakeLists.txt` - see `core/CMakeLists.txt`
for an example. 

The macro `add_test_exe` adds the executable to `make test` and places it in
`build/test` instead of `build/bin` when `-Dgroup_output=ON`

## Directories
    - build/ - contains output from components configured with
      -Dglobal_output_path
      - lib/ - (only when -Dgroup_output) 
      - bin/ - (only when -Dgroup_output) 
      - test/ - (only when -Dgroup_output)
    - cmake/ - cmake configuration macros
    - lib/ - lib sources common to all components
    - include/ - public headers declaring the interface of each module
    - app/ - application library component
      - include/ - private implementation detail headers
      - src/
      - test/
    - core/ - core platform services library component
      - src/
      - test/ - source files define compilation units for test executables and 
                  headers implement test bodies
      - lib/ - library sources used by only this component
      - build/ - contains output when not configured with -Dglobal_output_path
    - platform/ - platform-specific services implementations
      - ns3/ - NS3 simulator platform 
        - ext/
          - ns-3.21/ - NS3 source tarball contents
            - ns-3.21/ - NS3 source tree
              - build/ - the library and include directory for building NS3
                            simulations
                - ns3/ - NS3 public interface headers
                *.so - NS3 module libraries
    - scripts/ - misc. bash stuff

## Structure
    - core - *(library)* platform-independent abstractions of platform services
               like scheduling and messaging.
      - PlatformHost - abstract application container
      - Scheduler - asynchronous and delayed execution; guarantees that calling
                      will not block, but does not guarantee parallel execution.
      - Actor - base class for something that receives messages; actors can be in
                  the same process or on separate physical devices.
      - Messenger - message passing between actors
        - Channel - interface between message passing and physical transport;
                      sends and receives to and from an unspecified number of
                      endpoints of unspecified type (memory mapped file, socket,
                      bluetooth, etc.)
        - Socket - abstract IP socket 
      - UI
        - View - abstract displayable user interface
        - ViewModel - abstract data model displayed by View
        - ViewController - abstract ViewModel mutator
    - app - *(library)* platform-independent application logic; depends on core.
    - platform/native - *(library)* implements core services with POSIX or Win32
                          libraries; depends on core.
      - ThreadScheduler -> Scheduler
      - NativeChannel -> Channel
      - NativeSocket -> Socket
    - platform/ns3 - *(library)* implements core services by delegating to
                       the NS3 simulator; depends on core.
      - NsScheduler -> Scheduler
      - NsChannel -> Channel
      - NsSocket -> Socket
    - platform/android - *(library)* implements core services with Android NDK and
                           declares JNI interfaces for interoperating with Android
                           applications.

## Testing

Using gtest:

    test/test1.hh
    ------------------
    #pragma once

    #include "log.hh"
    #include "gtest/gtest.h"

    TEST(MyClass, it_does_x_correctly)
    {
      LOG(DEBUG) << "Hello!";
      ASSERT_EQ(1, 1);
      ASSERT_GT(1, 0);
      LOG(FATAL) << "Something terrible happened";
    }

    test/test_main.cc
    ------------------
    /**************************************************************************
     * This file just creates a compilation unit with the tests.
     *************************************************************************/
    #include "log.hh"
    // Must come first and only once in application 
    _INITIALIZE_EASYLOGGINGPP

    #include "test1.hh"
    // more tests

    #include "gtest/gtest.h"

    int main(int argc, char** argv)
    {
      // Allow command line arguments to specify tests
      ::testing::InitGoogleTest(&argc, argv);

      // Trim down some noisy output
      // Optionally turn logging off or change debug level (default ALL)
      el::Loggers::reconfigureAllLoggers(
        el::ConfigurationType::Format,
        "%datetime{%m:%s.%g} %levshort [%thread] %func (%fbase:%line) %msg");

      return RUN_ALL_TESTS();
    }

## Project Setup
### Using C++11
#### NS-3 via `wscript`

    def build(bld):
      obj = bld.create_ns3_program('ex', ['csma', 'internet', 'applications'])
      obj.cxxflags = ['-std=c++11']
      obj.source = ['ex.cc']
        
Compile and run by changing into the `ns-3.xx/build` directory and running
`./waf --run example`

#### Android
> If you're using command line NDK support this is what I had to put in my
> `jni/Application.mk` to get C++11 support with API 19

([stackoverflow](http://stackoverflow.com/a/21386866))

    NDK_TOOLCHAIN_VERSION := 4.8
    # APP_STL := stlport_shared  --> does not seem to contain C++11 features
    APP_STL := gnustl_shared
    
    # Enable c++11 extentions in source code
    APP_CPPFLAGS += -std=c++11


### README.txt

Be sure to clean the project by `make clean`.

Unit test includes:

    make testcontentdirectory // test the ranking mechanism

In class `DataLayer.cpp`, a posix-compatible platform-dependent API is used. To
port the project to other platform, e.g., windows, extra efforts are needed.

To test the whole system, a simple command sequence can be like this:

All timestamp should be UTC time, to save trouble, e.g., dst

    (1) create network adhoc
    (2) create device a
    (3) setgps a 50 -30
    (4) create device b
    (5) setgps b 50 -30.0017
    (6) create device c
    (7) setgps c 50 -30.0013
    (8) create device d
    (9) connect a
    (10) connect b
    (11) connect c
    (12) neighbor a
    (13) neighbor b
    (14) neighbor c
    (15) directory a
    (16) publish a ./media/beautiful.mp3 test.mp3
    (17) directory a
    (18) directory b
    (19) directory c
    (20) retrieve c test.mp3 ./meida/copy.mp3

to produce the output:

    emulation in batch mode: 5km x 5km spaces.
    
    (33 40 12 N ,112 11 07 W)
    latitude: 33.67011111111111
    longitude: -112.18530555555556
    
    latitude: 33.67011111111111
    longitude: -112.23944444444444
    
    latitude: 33.625
    longitude: -112.23944444444444
    
    latitude: 33.625
    longitude: -112.18530555555556
