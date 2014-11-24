depend(SMA_LIB)
depend(SMA_CCN_LIB)
depend(SMA_NS3_LIB)

project(${SMA_NS3_LIB})

# Configure cmake to find the NS3 headers and shared libraries for linking.
# Also defines the use_cmake_modules(target modules...) macro to link the
# modules by name (e.g. "wifi" becomes "libns3.21-wifi-debug.so")
include("cmake/ns3.cmake")

###############################################################################
# SOURCES

# The main library implements the abstract core services using NS3 facilities.
set(lib_sources
  "src/chrono.cpp"
  "src/ns3async.cpp"
  "src/ns3inetsocket.cpp"
  "src/link/ns3inetlink.cpp"
)

set(simulation_sources
  "src/ns3appcontainer.cpp"
)

###############################################################################
# TARGETS

# Since the library is static it's going to be linked based on what gets
# used. Since our dynamic dependencies will also depend on what gets used,
# we won't bother linking any modules to the library.
# Whatever simulation links to the library will have to make up the missing
# symbols by linking NS3 modules.
add_library("${PROJECT_NAME}" STATIC ${lib_sources})
target_link_libraries("${PROJECT_NAME}" "${SMA_LIB}")

add_executable("simulations" "sim/sim_main.cpp" ${simulation_sources})
target_link_libraries("simulations" "${SMA_CCN_LIB}" "${SMA_NS3_LIB}" "gtest")
# Unfortunately, the nice thing waf does for us is build up their
# configuration-encoded dependency hierarchy.
# We have to figure it out by adding stuff until we stop getting link errors.
use_ns3_modules("simulations"
  "core" "network" "internet" "csma" "wifi" "applications")
