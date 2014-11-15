PROJECT(smaplatform)

###############################################################################
# SOURCES

SET(sources
  "src/buffer.cpp"
  "src/buffer_view.cpp"

  "src/cyclic_barrier.cpp"
  "src/threadpool.cpp"
  "src/thread_scheduler.cpp"

  "src/message.cpp"
  "src/message_dispatch.cpp"

  "src/inet_addr.cpp"
  "src/channel.cpp"

  "src/node.cpp"

  "src/device/gps.cpp"
  "src/device/gps_dummy_device.cpp"
)


###############################################################################
# TARGETS

ADD_LIBRARY(${PROJECT_NAME} STATIC ${sources})
add_test_exe("platformlib_test" "test/test_main.cpp")
TARGET_LINK_LIBRARIES("platformlib_test" ${PROJECT_NAME})
