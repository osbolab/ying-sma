project("${SMA_ANDROID_CORE_LIB}")

###############################################################################
# SOURCES

set(sources
  "src/bsd_socket.cpp"
  "src/select_channel.cpp"
)


###############################################################################
# TARGETS

add_library("${PROJECT_NAME}" STATIC ${sources})
target_link_libraries("${PROJECT_NAME}" "${SMA_PLATFORM_LIB}")

add_test_exe("androidcore_test" "test/test_main.cc")
target_link_libraries("androidcore_test" "${PROJECT_NAME}")
