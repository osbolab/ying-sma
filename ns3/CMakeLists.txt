project(smans3)

# Configure cmake to find the NS3 headers and shared libraries for linking.
# Also defines the use_cmake_modules(target modules...) macro to link the
# modules by name (e.g. "wifi" becomes "libns3.21-wifi-debug.so")
include("cmake/ns3.cmake")

###############################################################################
# SOURCES

# The main library implements the abstract core services using NS3 facilities.
set(sources
  "src/time.cpp"
  "src/ns3_socket.cpp"
  "src/ns3_channel.cpp"
  "src/ns3_scheduler.cpp"
)

###############################################################################
# TARGETS

# Since the library is static it's going to be linked based on what gets
# used. Since our dynamic dependencies will also depend on what gets used,
# we won't bother linking any modules to the library.
# Whatever simulation links to the library will have to make up the missing
# symbols by linking NS3 modules.
add_library("smans3" STATIC ${sources})
target_link_libraries("smans3" "smacore")

add_executable("wifi_simple" "sim/wifi_simple.cpp" "src/app_container.cpp")
target_link_libraries("wifi_simple" "smaapp" "smans3")
# Unfortunately, the nice thing waf does for us is build up their
# configuration-encoded dependency hierarchy.
# We have to figure it out by adding stuff until we stop getting link errors.
use_ns3_modules("wifi_simple"
  "core" "network" "internet" "csma" "applications")
