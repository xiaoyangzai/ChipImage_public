# CMakeList.txt : CMake project for cutImageAlgr, include source and define
# project specific logic here.
#
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add source to this project's executable.
file (GLOB LIBRARY_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library (image_process SHARED ${LIBRARY_SRC})
if(OpenCV_FOUND)
    # Add OpenCV header files top project
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    # Add OpenCV library to project
    target_link_libraries(image_process ${OpenCV_LIBS} Crypt32)
endif()

set(VERSION_CONTENT "${GIT_COMMIT_HASH}\n")

file(WRITE "${PROJECT_SOURCE_DIR}/bin/version.txt" "${VERSION_CONTENT}")

set_target_properties(image_process PROPERTIES
    OUTPUT_NAME "image_process"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET image_process PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
add_custom_command(TARGET image_process POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:image_process> ${PROJECT_SOURCE_DIR}/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/image.jpg ${PROJECT_SOURCE_DIR}/bin/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/target.jpg ${PROJECT_SOURCE_DIR}/bin/)

add_custom_target(CopyFiles ALL DEPENDS unit_test image_process)