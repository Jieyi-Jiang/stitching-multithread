cmake_minimum_required(VERSION 3.15)

# set the compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)


# set the project name
if(NOT CMAKE_BUILD_TYPE)
#   set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_BUILD_TYPE Debug)
endif()

############################add OpenCV ############################
# Adjust the path to your OpenCV installation
set(OpenCV_DIR "/home/jieyi/projects/opencv-lib/install/")
find_package( OpenCV REQUIRED )
include_directories( 
    ${OpenCV_INCLUDE_DIRS} 
)

function(make_one_executable target source)
    message(STATUS "Building ${target} with source ${source}")
    # message(WARNING "Building ${target} with source ${source}")
    add_executable(${target} ${source})
    target_link_libraries(${target} ${OpenCV_LIBS})
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/test/${target} ${CMAKE_SOURCE_DIR}/excutable
)
endfunction()


make_one_executable(stitching_demo stitching_demo.hpp)

make_one_executable(check_camera ./check_camera.cpp)

make_one_executable(stitching ./stitching.cpp)

set(SOURCES ../src/camera.cpp  ./test_camera_class.cpp)
make_one_executable(test_camera_class "${SOURCES}")

make_one_executable(read_3_camera ./read_3_camera.cpp)