# CMakeList.txt : CMake project for cutImageAlgr, include source and define
# project specific logic here.
#
# Add source to this project's executable.
file (GLOB LIBRARY_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library (Myimageops SHARED ${LIBRARY_SRC})
if(OpenCV_FOUND)
    # Add OpenCV header files top project
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    # Add OpenCV library to project
    target_link_libraries(Myimageops ${OpenCV_LIBS} Crypt32)
endif()

set_target_properties(Myimageops PROPERTIES
    OUTPUT_NAME "Myimageops"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Myimageops PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
