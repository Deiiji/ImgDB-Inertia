project( imgdb-dbg )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Debug )
add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	)
set ( imgdb-dbg_HDRS
	bloom_filter.h
	haar.h
	imgdb.h
	)

set ( imgdb-dbg_SRCS
	bloom_filter.cpp
	haar.cpp
	)

add_library ( imgdb-dbg SHARED ${imgdb-dbg_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( imgdb-dbg  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
