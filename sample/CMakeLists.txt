file(GLOB SOURCES "*.cpp")

add_executable(unit_test ${SOURCES})

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/imageProcessAlgor)
if(OpenCV_FOUND)
    # Add OpenCV header files top project
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    # Add OpenCV library to project
    target_link_libraries(unit_test ${OpenCV_LIBS})
endif()
# 链接所需的库文件
target_link_libraries(unit_test Myimageops)

set_target_properties(unit_test PROPERTIES
    OUTPUT_NAME "unit_test"
)