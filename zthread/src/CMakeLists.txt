add_definitions(-DZTHREAD_EXPORTS)

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.cc
)

add_library(zthread SHARED ${SRCS})


