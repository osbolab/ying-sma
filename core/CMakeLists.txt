PROJECT(smacore)

###############################################################################
# SOURCES

SET(sources
  "src/byte_buf.cpp"
  "src/byte_view.cpp"

  "src/cyclic_barrier.cpp"
  "src/threadpool.cpp"

  "src/message.cpp"
  "src/messenger.cpp"

  "src/inet_addr.cpp"
)


###############################################################################
# TARGETS

ADD_LIBRARY(${PROJECT_NAME} STATIC ${sources})
add_test_exe("${PROJECT_NAME}_test" "test/test_main.cpp")
TARGET_LINK_LIBRARIES("${PROJECT_NAME}_test" ${PROJECT_NAME})
