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

############################add OpenCV ############################
# Adjust the path to your OpenCV installation
set(OpenCV_DIR "/home/jieyi/projects/opencv-lib/install/")
find_package( OpenCV REQUIRED )
include_directories( 
    ${OpenCV_INCLUDE_DIRS} 
)

function(make_one_executable target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} ${OpenCV_LIBS})
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/test/${target} ${CMAKE_SOURCE_DIR}/excutable
    )
endfunction()


make_one_executable(stitching_demo ./stitching_demo.cpp)

# project(test_camera)
# add_executable(test_camera ./test_camera.cpp)
# target_link_libraries( test_camera ${OpenCV_LIBS} )
# add_custom_command(TARGET test_camera POST_BUILD COMMAND ${CMAKE_COMMAND} -E
#   copy ${CMAKE_BINARY_DIR}/test_camera ${CMAKE_SOURCE_DIR}
# )

make_one_executable(test_camera ./test_camera.cpp)

make_one_executable(stitching ./stitching.cpp)


