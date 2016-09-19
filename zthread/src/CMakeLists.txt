# Allow the source files to find headers in src/
#include_directories(${POJRECT_SOURCE_DIR}/src)

add_definitions(-DZTHREAD_EXPORTS)

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.cxx
    ${PROJECT_SOURCE_DIR}/src/*.h)

#if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
# file(GLOB SRCS
#    ${SRCS}
#    ${PROJECT_SOURCE_DIR}/src/win32/*.cxx
#    ${PROJECT_SOURCE_DIR}/src/win32/*.h)
#endif()

add_library(zthead SHARED ${SRCS})


