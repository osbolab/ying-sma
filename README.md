## Building

Configure the makefile by invoking `cmake` in the build directory.
Do not invoke `cmake` in any component's root directory or any other path.

     # -DCMAKE_BUILD_TYPE=Debug - attach symbols to the output (on by default)
     # -Dbuild_tests=OFF        - don't build test executables

     cd core/build/
     cmake ../
     make
     ./libsma_test

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

Some consumer *G* generates an interest *I_T* in content type *T*.
She broadcasts this interest to her nearest neighbors, and they in turn
broadcast the interest if they have not already.

Each node that sees this interest inspects her *active interest table* (AIT) for
a match on *T*. The AIT maps *T* to some future time point.

If she finds an entry in her AIT she increments the mapped time by the interest
expiry duration *t*; if she finds no entry then she adds it with an initial
value of *t*.

Once every node in the network has seen *I_T* and has an entry in her AIT for
*T*`->`*t* the interest is *saturated* and the announcement phase is complete.

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
