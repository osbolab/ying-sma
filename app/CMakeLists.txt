project(smaapp)

###############################################################################
# SOURCES

set(sources
  "src/md5.cpp"
  "src/json.cpp"
  "src/split_regroup.cpp"

  "src/echoclient.cpp"
  "src/echoserver.cpp"

  "src/emulator.cpp"
  "src/batchscriptgenerator.cpp"

  "src/devicewithgps.cpp"
  "src/devicelogger.cpp"

  "src/datablock.cpp"
  "src/adhocnetworkemulator.cpp"

  "src/plainchunkstore.cpp"
  "src/pendingchunkmanager.cpp"
  "src/flowtable.cpp"
  "src/datalayer.cpp"

  "src/signalhandler.cpp"
  "src/segmenter.cpp"
  "src/pendingfilemanager.cpp"
  "src/neighborrecords.cpp"
  "src/neighborrecordentry.cpp"
  "src/gpsdriver.cpp"
  "src/controllayer.cpp"
  "src/contentdiscription.cpp"
  "src/contentdirectory.cpp"
)


###############################################################################
# TARGETS

# Depend on the core library project
add_library("${PROJECT_NAME}" STATIC ${sources})
target_link_libraries("${PROJECT_NAME}" "smacore")

add_test_exe("${PROJECT_NAME}_test" "test/test_main.cpp")
  

