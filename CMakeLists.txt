cmake_minimum_required(VERSION 3.0)
project(UVCURL C CXX)
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")



include_directories(${PROJECT_SOURCE_DIR})

find_package(CURL REQUIRED VERSION 7.10)
find_package(Libuv)
include_directories(${Libuv_INCLUDE_DIR})

add_subdirectory(uvcurl)
add_subdirectory(test)