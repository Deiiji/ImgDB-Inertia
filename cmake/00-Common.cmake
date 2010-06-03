add_definitions ( -Wno-write-strings ) #tell GCC to shut up about deprecated string conversions
										#also I should take this out later.
macro(copy_test_images)
	set(IMAGE_SOURCE ${CMAKE_SOURCE_DIR}/tests/testimages)
	set(DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/testimages)

  add_custom_target(copy_testimages ALL
    COMMENT "Copying files: ${IMAGE_SOURCE}")

  add_custom_command(
    TARGET copy_testimages
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${IMAGE_SOURCE} ${DESTINATION}
    )
endmacro(copy_test_images)

macro(copy_test_scripts)
	set(GLOBPAT ${CMAKE_SOURCE_DIR}/tests/*.sh)
	set(DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  file(GLOB COPY_FILES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLOBPAT})
  add_custom_target(copy_testscripts ALL
    COMMENT "Copying files: ${GLOBPAT}")

  foreach(FILENAME ${COPY_FILES})
    set(SRC "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}")
    set(DST "${DESTINATION}/${FILENAME}")

    add_custom_command(
      TARGET copy_testscripts
      COMMAND ${CMAKE_COMMAND} -E copy ${SRC} ${DST}
      )
  endforeach(FILENAME)
endmacro(copy_test_scripts)
