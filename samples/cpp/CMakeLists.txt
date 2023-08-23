file(GLOB SOURCES "*.cpp")

add_executable(unit_test ${SOURCES})

include_directories(${PROJECT_SOURCE_DIR}/imageProcessAlgor)
if(OpenCV_FOUND)
    # Add OpenCV header files top project
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    # Add OpenCV library to project
    target_link_libraries(unit_test ${OpenCV_LIBS})
endif()
target_link_libraries(unit_test image_process)

set_target_properties(unit_test PROPERTIES
    OUTPUT_NAME "unit_test"
)

add_custom_command(TARGET unit_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unit_test> ${PROJECT_SOURCE_DIR}/bin/)