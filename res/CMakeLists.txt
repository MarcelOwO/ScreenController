add_custom_target(
  copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "$<TARGET_FILE_DIR:ScreenController>/assets"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}"
          "$<TARGET_FILE_DIR:ScreenController>/assets"
  VERBATIM)
