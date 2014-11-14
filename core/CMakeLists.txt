PROJECT(smacore)

###############################################################################
# SOURCES

SET(sources
  "src/byte_buf.cpp"
  "src/byte_view.cpp"

  "src/cyclic_barrier.cpp"
  "src/threadpool.cpp"
  "src/thread_scheduler.cpp"

  "src/message.cpp"
  "src/messenger.cpp"

  "src/inet_addr.cpp"
  "src/channel.cpp"

  "src/node.cpp"
)


###############################################################################
# TARGETS

ADD_LIBRARY(${PROJECT_NAME} STATIC ${sources})
add_test_exe("corelib_test" "test/test_main.cpp")
TARGET_LINK_LIBRARIES("corelib_test" ${PROJECT_NAME})
