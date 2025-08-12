cmake_minimum_required(VERSION 3.5)
project( video_stitching )
set(CMAKE_CXX_STANDARD 17) 

set(OpenCV_DIR "/home/jieyi/projects/opencv-lib/build/")
find_package( OpenCV REQUIRED )
include_directories( 
    ${OpenCV_INCLUDE_DIRS} 
)

include_directories(
    ./src
)
add_executable( video_stitching 
    ./src/main.cpp 
    ./src/Camera_input.cpp 
    ./src/Est_exposure.cpp 
    ./src/Est_seam.cpp 
    ./src/KeyPoint.cpp 
    ./src/Stitch_frame.cpp )

target_link_libraries( video_stitching ${OpenCV_LIBS} )

