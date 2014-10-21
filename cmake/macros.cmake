############################################
# OBJECTS

macro(object OBJ GROUP)
  file(GLOB O_${OBJ} ${ARGN})
  source_group("main\\${GROUP}" FILES ${O_${OBJ}})
endmacro(object)

macro(test_object OBJ GROUP)
  file(GLOB O_${OBJ}_test ${ARGN})
  source_group("test\\${GROUP}" FILES ${O_${OBJ}_test})
endmacro(test_object)


############################################
# TESTING            (cmake -Dbuild_tests=ON)

if (build_tests)
  message               (STATUS "Building test projects")
  add_subdirectory      (${LIB_GTEST})
  include_directories   (${gtest_SOURCE_DIR}/include)
  set_target_properties (gtest       PROPERTIES FOLDER ${FOLDER_TEST})
  set_target_properties (gtest_main  PROPERTIES FOLDER ${FOLDER_TEST})

  macro(add_test OBJ)
    set(T_${OBJ} ${O_${OBJ}})
    foreach(OBJ_NAME ${ARGN})
      set(T_${OBJ} ${T_${OBJ}} ${O_${OBJ_NAME}})
    endforeach()
    add_executable        (${OBJ}_test ${O_${OBJ}_test} ${T_${OBJ}})
    target_link_libraries (${OBJ}_test gtest_main)
    set_target_properties (${OBJ}_test PROPERTIES FOLDER ${FOLDER_TEST})
    message(STATUS "Added test: ${OBJ}")
  endmacro(add_test)
else()
  macro(add_test)
  endmacro(add_test)
endif()

macro(add_exe EXE)
  set(OBJS_${EXE} ${O_${EXE}})
  foreach(OBJ_NAME ${ARGN})
    set(OBJS_${EXE} ${OBJS_${EXE}} ${O_${OBJ_NAME}})
  endforeach()
  add_executable(${EXE} ${OBJS_${EXE}})
  message(STATUS "Added executable: ${EXE}")
endmacro(add_exe)