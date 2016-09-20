# Allow the source files to find headers in src/
#include_directories(${POJRECT_SOURCE_DIR}/src)

add_definitions(-DZTHREAD_EXPORTS)
set(CMAKE_CXX_FLAGS "-fpermissive")

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.cxx
    ${PROJECT_SOURCE_DIR}/src/*.h)

add_library(zthead SHARED ${SRCS})


