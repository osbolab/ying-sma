depend(SMA_LIB)
depend(SMA_CCN_LIB)

project("${SMA_CCN_LIB}")

###############################################################################
# SOURCES

set(sources
  "src/ccnapplication.cpp"

  "src/md5.cpp"
  "src/json.cpp"

  "src/devicewithgps.cpp"
  "src/devicelogger.cpp"

  "src/datablock.cpp"

  "src/plainchunkstore.cpp"
  "src/pendingchunkmanager.cpp"
  "src/flowtable.cpp"
  "src/datalayer.cpp"

  "src/signalhandler.cpp"
  "src/segmenter.cpp"
  "src/pendingfilemanager.cpp"
  "src/neighborrecords.cpp"
  "src/neighborrecordentry.cpp"
  "src/controllayer.cpp"
  "src/contentdescriptor.cpp"
  "src/contentdirectory.cpp"
)


###############################################################################
# TARGETS

# Depend on the platform interfaces library 
add_library("${PROJECT_NAME}" STATIC ${sources})
target_link_libraries("${PROJECT_NAME}" "${SMA_LIB}")

add_test_exe("ccnlib_test" "test/test_main.cpp" "test/chrono.cpp")
target_link_libraries("ccnlib_test" "${PROJECT_NAME}")
  

