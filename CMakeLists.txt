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
set(CMAKE_CXX_STANDARD 17) 

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
file(GLOB C_SOURCES ./src/*.c)
file(GLOB CPP_SOURCES ./src/*.cpp)
set(SOURCES ${C_SOURCES} ${CPP_SOURCES})
add_executable( ${PROJECT} ${SOURCES} )

target_link_libraries( ${PROJECT} ${OpenCV_LIBS} )

# Copy the target executable at the right location.
add_custom_command(TARGET ${PROJECT} POST_BUILD COMMAND ${CMAKE_COMMAND} -E
  copy ${CMAKE_BINARY_DIR}/${PROJECT} ${CMAKE_SOURCE_DIR}
)
