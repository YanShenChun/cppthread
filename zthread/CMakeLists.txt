cmake_minimum_required(VERSION 2.8.11)
project(zthread)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DZT_WIN32 -DUNICODE -D_UNICODE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DZT_POSIX)
else()
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
add_subdirectory(src)
