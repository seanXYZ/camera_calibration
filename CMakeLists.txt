project(tool_calibrate)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_COMPILER   gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_FLAGS     "${CMAKE_C_FLAGS}   -O3")
set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -O3")

set(TARGET "tool_calibrate")

include_directories("${PROJECT_SOURCE_DIR} .")
aux_source_directory(. SRC_FILE)

message(STATUS "source file" ${SRC_FILE})

add_executable(${TARGET} ${SRC_FILE})
target_link_libraries(${TARGET} "opencv_core")
target_link_libraries(${TARGET} "opencv_highgui")
target_link_libraries(${TARGET} "opencv_features2d")
target_link_libraries(${TARGET} "opencv_calib3d")
target_link_libraries(${TARGET} "opencv_imgproc")

#EXECUTABLE_OUTPUT_PATH && LIBRARY_OUTPUT_PATH用来指定二进制文件位置和库文件路径，这里不包括中间文件  
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)  
