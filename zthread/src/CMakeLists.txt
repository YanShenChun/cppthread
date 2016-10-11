# Allow the source files to find headers in src/
#include_directories(${POJRECT_SOURCE_DIR}/src)

add_definitions(-DZTHREAD_EXPORTS)

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.cc
)

add_library(zthread SHARED ${SRCS})


