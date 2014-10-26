### C++11
#### NS-3 via `wscript`

    def build(bld):
        obj = bld.create_ns3_program('example', ['csma', 'internet', 'applications'])
        obj.cxxflags = ['-std=c++11']
        obj.source = ['example.cc']
        
Compile and run by changing into the `ns-3.xx/build` directory and running `./waf --run example`

#### Android
> If you're using command line NDK support this is what I had to put in my `jni/Application.mk` to
> get C++11 support with API 19

([stackoverflow](http://stackoverflow.com/a/21386866))

    NDK_TOOLCHAIN_VERSION := 4.8
    # APP_STL := stlport_shared  --> does not seem to contain C++11 features
    APP_STL := gnustl_shared
    
    # Enable c++11 extentions in source code
    APP_CPPFLAGS += -std=c++11

### Sockets Layer
#### Berkely Sockets
    #ifdef __WIN32__
    # include <winsock2.h>
    # else
    # include <sys/socket.h>
    #endif

##### Initialization

    #ifdef __WIN32__
      WORD versionWanted = MAKEWORD(1, 1);
      WSADATA wsaData;
      int success = WSAStartup(versionWanted, &wsaData);
      assert(success != 0 && "Winsock version not supported");
    #endif

    socket()

#### NS3