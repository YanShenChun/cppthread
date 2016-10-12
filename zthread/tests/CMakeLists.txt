
#add_definitions(-DZTHREAD_EXPORTS)

file(GLOB SRCS
    ${PROJECT_SOURCE_DIR}/tests/*.cc
)

add_executable(demo ${SRCS})

set(DEMO_DEPENDS "zthread")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DEMO_DEPENDS ${DEMO_DEPENDS} pthread)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DEMO_DEPENDS ${DEMO_DEPENDS} pthread)
else()
endif()


target_link_libraries(demo ${DEMO_DEPENDS})


