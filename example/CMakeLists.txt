FILE(GLOB_RECURSE UT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c)
SET (TO_DEL_CPP_LIST
	stub.cpp
)
list(REMOVE_ITEM UT_SOURCES ${TO_DEL_CPP_LIST})


SET(EXAMPLE_TARGET _${EXAMPLE_BINARY})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

ADD_EXECUTABLE(${EXAMPLE_TARGET} ${UT_SOURCES})

LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)

TARGET_LINK_LIBRARIES(${EXAMPLE_TARGET} ${PROJECT_NAME} event event_pthreads pthread)

SET_TARGET_PROPERTIES(${EXAMPLE_TARGET} PROPERTIES OUTPUT_NAME ${EXAMPLE_BINARY})

ADD_DEFINITIONS(-DDBW_TEST)

