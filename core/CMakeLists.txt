PROJECT(smacore)

###############################################################################
# SOURCES

SET(sources
  "src/buffer.cpp"
  "src/buffer_view.cpp"

  "src/message.cpp"
  "src/channel.cpp"
  "src/message_dispatch.cpp"

  "src/inet_addr.cpp"
)


###############################################################################
# TARGETS

ADD_LIBRARY(${PROJECT_NAME} STATIC ${sources})
add_test_exe("corelib_test" "test/test_main.cpp")
TARGET_LINK_LIBRARIES("corelib_test" ${PROJECT_NAME})
