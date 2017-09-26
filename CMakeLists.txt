CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

PROJECT(keepassxc-proxy-cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
set(Boost_USE_STATIC_LIBS        OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
set(BOOST_ALL_DYN_LINK           ON)
message("Boost include dir = " "${Boost_INCLUDE_DIR}")
message("Boost library dir = " "${Boost_LIBRARY_DIRS}")

add_executable(keepassxc-proxy keepassxc-proxy.cpp)
target_link_libraries(keepassxc-proxy ${Boost_LIBRARIES})