depend(SMA_LIB)
depend(SMA_CCN_LIB)

project("${SMA_CCN_LIB}")

###############################################################################
# SOURCES

set(sources
  "src/ccnnode.cpp"
  "src/ccnmessage.cpp"
)


###############################################################################
# TARGETS

# Depend on the platform interfaces library 
add_library("${PROJECT_NAME}" STATIC ${sources})
target_link_libraries("${PROJECT_NAME}" "${SMA_LIB}")

add_test_exe("ccnlib_test" "test/test_main.cpp" "test/chrono.cpp")
target_link_libraries("ccnlib_test" "${PROJECT_NAME}")
  

