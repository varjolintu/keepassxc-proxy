cmake_minimum_required(VERSION 3.1.0)

project(keepassxc-proxy)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
find_package(Qt5Network)

set(proxy_SOURCES
       main.cpp
       NativeMessagingHost.cpp
    )

#add_executable(${PROJECT_NAME} "main.cpp")
add_executable(${PROJECT_NAME} ${proxy_SOURCES})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network)
