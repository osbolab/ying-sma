`include/ns3/` and `lib/` should be symlinked to the NS3 build directory where
NS3 builds the shared libraries and outputs the headers for the modules its
built with.

These paths will be `include/ns3/` will be added to the system include path and
`lib/` to the library search path for targets in this project. Source depending
on NS3 can therefore include its headers as if they were in the system path:

    #include <ns3/xxxx.h>

This means that NS3 needs to be built before any source depending on it or those
headers won't be found.

If NS3 is moved or the version changed then the symlinks need to be updated.
