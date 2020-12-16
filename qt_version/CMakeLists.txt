cmake_minimum_required(VERSION 3.1.0)

project(keepassxc-proxy)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

macro(add_gcc_compiler_cxxflags FLAGS)
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
  endif()
endmacro(add_gcc_compiler_cxxflags)

add_gcc_compiler_cxxflags("-std=c++11")

if(APPLE)
  add_gcc_compiler_cxxflags("-stdlib=libc++")
endif()

find_package(Qt5 COMPONENTS Core Network)

set(proxy_SOURCES
       main.cpp
       NativeMessagingProxy.cpp
    )

#add_executable(${PROJECT_NAME} "main.cpp")
add_executable(${PROJECT_NAME} ${proxy_SOURCES})

target_link_libraries(${PROJECT_NAME} Qt::Core Qt::Network)
