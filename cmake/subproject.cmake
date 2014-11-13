if(subproject_cmake_included)
  return()
endif()
set(subproject_cmake_included ON)

# Enable selective project configuration from the root directory.
# -DPROJECT=ON configures that project and -DALL=ON configures everything.
# If nothing is enabled then enable everything
set(subproject_count 0)
macro(subproject name)
  if(${name} OR ALL)
    message(STATUS "\nConfiguring ${name}")
    add_subdirectory(${name} ${ARGN})
    message(STATUS "Leaving ${name}")
    math(EXPR subproject_count "${subproject_count} + 1")
  endif()
endmacro()
