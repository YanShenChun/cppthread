
#add_definitions(-DZTHREAD_EXPORTS)

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/tests/*.cc
)

add_executable(demo ${SRCS})
target_link_libraries(demo zthread pthread)


