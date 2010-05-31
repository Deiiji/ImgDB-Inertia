cmake_minimum_required( VERSION 2.6 )

project( imgdb-test )
set ( CMAKE_BUILD_TYPE Debug )

add_definitions ( -Wall )

find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
include (00-Common)

include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	../ImgDB)
	
link_directories( ../ImgDB )
link_libraries( imgdb-dbg )

set ( imgdb-test_HDRS
	bloom_filter.h
	haar.h
	imgdb.h
	)

set ( imgdb-test_SRCS
	test.cpp
	)

add_executable ( imgdb-test ${imgdb-test_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( imgdb-test  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
