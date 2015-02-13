depend(SMA_LIB)

project("${SMA_ANDROID_LIB}")

###############################################################################
# SOURCES

set(sources
  "src/bsd_socket.cpp"
  "src/select_channel.cpp"
)


###############################################################################
# TARGETS

add_library("${PROJECT_NAME}" STATIC ${sources})
target_link_libraries("${PROJECT_NAME}" "${SMA_LIB}")
