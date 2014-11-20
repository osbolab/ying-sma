function(publish_headers)
  foreach(dir IN LISTS ARGN)
    file(GLOB_RECURSE headers RELATIVE "${PROJECT_SOURCE_DIR}" "${dir}/*")
    set(published_headers "")
    foreach(header IN LISTS headers)
      set(header_file "${PROJECT_SOURCE_DIR}/${header}")
      set(output "${CMAKE_BINARY_DIR}/${header}")
      list(APPEND published_headers "${output}")
      add_custom_command(
        OUTPUT "${output}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${header_file}" "${output}"
        MAIN_DEPENDENCY "${header_file}"
        COMMENT "Publishing ${header}"
        VERBATIM
      )
    endforeach()
  endforeach()
  add_custom_target(
    publish_headers
    ALL
    DEPENDS ${published_headers}
    SOURCES ${headers}
  )
endfunction()
