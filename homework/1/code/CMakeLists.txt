cmake_minimum_required(VERSION 3.17)

project(code)

# set(CMAKE_CXX_STANDARD 11)

add_executable(code main.cpp rasterizer.hpp rasterizer.cpp Triangle.hpp Triangle.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(/usr/local/include)

# Where to find CMake modules and OpenCV

set(OpenCV_DIR "E://opencv//graphics_lib_cpp//opencv_2//opencv//mingw64_build//install")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# add libs you need

set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)

# linking

target_link_libraries(code ${OpenCV_LIBS})

#eigen库

include_directories(E://opencv//computer graphics//eigen//eigen3)

set(SOURCE_FILES main.cpp)
