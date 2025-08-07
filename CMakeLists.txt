cmake_minimum_required(VERSION 3.5)

# set the compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# set the project name
if(NOT CMAKE_BUILD_TYPE)
#   set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(PROJECT video_stitching)
project( ${PROJECT} )

############################add OpenCV ############################
# Adjust the path to your OpenCV installation
set(OpenCV_DIR "/home/jieyi/projects/opencv-lib/install/")
find_package( OpenCV REQUIRED )
include_directories( 
    ${OpenCV_INCLUDE_DIRS} 
)

########################## add inc and src ########################
include_directories(
    ./include
)
file(GLOB C_SOURCES ./source/*.c)
file(GLOB CPP_SOURCES ./source/*.cpp)
set(SOURCES ${C_SOURCES} ${CPP_SOURCES}
        test/MyStitcher.cpp
        test/MyStitcher.h)
add_executable( ${PROJECT} ${SOURCES} )

target_link_libraries( ${PROJECT} ${OpenCV_LIBS} )

# Copy the target executable at the right location.
add_custom_command(TARGET ${PROJECT} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
  copy ${CMAKE_BINARY_DIR}/${PROJECT} ${CMAKE_SOURCE_DIR}/excutable
)


add_subdirectory(./test)

# ########################## stitching_demo #########################
# project(stitching_demo)
# add_executable(stitching_demo ./test/stitching_demo.cpp)
# target_link_libraries( stitching_demo ${OpenCV_LIBS} )
# add_custom_command(TARGET stitching_demo POST_BUILD COMMAND ${CMAKE_COMMAND} -E
#   copy ${CMAKE_BINARY_DIR}/stitching_demo ${CMAKE_SOURCE_DIR}
# )

# ########################## test_camera #########################
# project(test_camera)
# add_executable(test_camera ./test/test_camera.cpp)
# target_link_libraries( test_camera ${OpenCV_LIBS} )
# add_custom_command(TARGET test_camera POST_BUILD COMMAND ${CMAKE_COMMAND} -E
#   copy ${CMAKE_BINARY_DIR}/test_camera ${CMAKE_SOURCE_DIR}
# )