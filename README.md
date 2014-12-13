## Building

### Environment

    uname -rps
    # Linux 3.16.0-28-generic x86_64

    cmake --version
    # cmake version 2.8.12.2

    c++ --version
    # Ubuntu clang version 3.6.0-svn224099-1~exp1 (trunk) (based on LLVM 3.6.0)
    # Target: x86_64-pc-linux-gnu
    # Thread model: posix

    python --version
    # Python 2.7.8

### NS3

The sma-ns3 project depends on the NS3 module libraries and headers, so you'll
need to build them first. NS3 packages its own build tool, so just use that.

You need at least python2.7 to build NS3.

From the project root:

    cd ns3/ext/ns3/
    ./build.py

From there, check that the module libraries and headers were output to the right
directory:

    cd ns-3.21/build
    ls *.so | wc -l
    # output: 39
    ls ns3/*.h | wc -l
    # output: 795

### SMA

Configure the makefile by invoking `cmake` in the build directory.
Build files are output to the current directory when running cmake, so make
sure you're always in a subdirectory first.

Cmake finds all of the source files and generates makefiles. You only need to
run it once (or whenever you delete the build dir), after which you should run
`make` to build the targets. The makefile automatically updates itself when 
source files are added or removed to or from the `CMakelists.txt` file.

Cmake generates subdirectories in the build path for the subprojects, like
the sma-ns3 library. Targets for that project are output to its build
subdirectory so you'll need to change into that path before you can run the
executables.

    # -DCMAKE_BUILD_TYPE=Debug - attach symbols to the output (on by default)
    # -Dbuild_tests=OFF        - don't build test executables

    mkdir build
    cd build/
    cmake ../
    make
    cd ns3/
    # Running with no options creates 2 nodes at a distance of 2000m.
    ./wifi-test --nodes=2 --distance=100

Project sources are declared in `CMakeLists.txt`.

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

## C++11

### Android
([stackoverflow](http://stackoverflow.com/a/21386866))

`jni/Application.mk` 

    NDK_TOOLCHAIN_VERSION := 4.8
    # APP_STL := stlport_shared  --> does not seem to contain C++11 features
    APP_STL := gnustl_shared
    
    # Enable c++11 extentions in source code
    APP_CPPFLAGS += -std=c++11

## Protocols

### Content dissemination

1. Interest
  a. Announcement
  a. Replication
1. Content
  a. Announcement
  a. Request
  a. Distribution
  a. Caching

#### Interest

##### Announcement

Some consumer is interested in a Content Type T. She places this in her Local
Interest Table (LIT), a mapping from Content Type to Rank.
She disseminates her interest by periodically selecting entries from her LIT
and broadcasting those to her neighbors bearing the label "original interest,"
so as to distinguish this broadcast from later replication broadcasts.
An original interest is rebroadcast regardless of the state of the network to
bring it to the next good known state.

Each neighbor who receives that broadcast will add all of her interests to his
Remote Interset Table (RIT), a mapping from Content Type to a time point some
distance in the future. If the received interest is not in his RIT it is added;
otherwise he updates the mapped time point to further in the future relative to
the current time. He will periodically cull from the table any interests that
map to a time point in the past, thus enforcing that only relevant interests
persist in the network.

When any neighbor has a nonempty LIT or RIT he should periodically select from
each some interests to disseminate and broadcast them to his neighbors.  To
minimize redundant broadcasts, each node should also maintain the set of its
neighbors from which it has not seen a particular interest. It must continue
rebroadcasting any interest with a nonempty set until the set is cleared by
observing neighbors broadcast the interest. This way interests will flood
outward from their origin nodes and, once the network reaches equilibrium,
updates will only occur around newly arriving nodes.

##### Replication

When a node arrives to the network he announces his arrival to his nearest
neighbors. Each of them responds by broadcasting their interest tables to
him until he has a complete table. He uses the mapped time values stored by
the other nodes and does not increment them.

#### Content

Some producer *P* generates a content *C_T* of type *T* and computes its hash
*H*.
She segments this content into *N* blocks *L* bytes long and indexes them from
0 to *N*-1. She then names each block *B_i* by concatenating the
hash value of the original content with the block's index and hashing that
concatenated value. Finally she stores these blocks in her local cache, thus
becoming a *seed* for *C_T*.

##### Announcement

When the producer *P* has finished generating the blocks for her content she
will notify her immediate neighbors that she has new content with type *T* and
that its name is *H*, the original content's hash value.

##### Request

##### Distribution

##### Caching
