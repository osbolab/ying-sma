project("${SMA_PLATFORM_LIB}")

###############################################################################
# SOURCES

set(sources
  "src/buffer.cpp"
  "src/buffer_view.cpp"

  "src/actor.cpp"

  "src/message.cpp"
  "src/channel.cpp"
  "src/message_dispatch.cpp"

  "src/inet_addr.cpp"
)

###############################################################################
# TARGETS

# publish_headers("include/sma")

add_library(${PROJECT_NAME} STATIC ${sources})
add_dependencies(${PROJECT_NAME} publish_headers)

add_test_exe("platformlib_test" "test/test_main.cpp")
target_link_libraries("platformlib_test" ${PROJECT_NAME})
