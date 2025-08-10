cmake_minimum_required(VERSION 3.5)

# set the compiler
set(CMAKE_CXX_STANDARD 23)
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
set(OpenCV_DIR "/home/jieyi/projects/opencv-lib/install/lib/cmake/opencv4")
find_package( OpenCV REQUIRED )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")
include_directories( 
    ${OpenCV_INCLUDE_DIRS} 
)

############################ add yaml-cpp ############################
add_subdirectory(./third-party/yaml-cpp)

########################## add inc and src ########################
include_directories(
    ./include
    ./third-party
)
file(GLOB C_SOURCES ./source/*.c)
file(GLOB CPP_SOURCES ./source/*.cpp)
set(SOURCES 
  ${C_SOURCES} 
  ${CPP_SOURCES}
)
add_executable( ${PROJECT} ${SOURCES} )

target_link_libraries( ${PROJECT} PRIVATE ${OpenCV_LIBS} )
target_link_libraries( ${PROJECT} PRIVATE yaml-cpp )

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