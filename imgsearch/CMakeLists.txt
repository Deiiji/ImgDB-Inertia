cmake_minimum_required( VERSION 2.6 )

project( imgsearch )
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
	
set ( imgsearch_HDRS
	
	)

set ( imgsearch_SRCS
	main.cpp
	mainwindow.cpp
	)

set ( imgsearch_UIS
	mainwindow.ui
	)
QT4_WRAP_UI(UIS ${imgsearch_UIS})

set ( imgsearch_MOCS
	mainwindow.h
	)
QT4_WRAP_CPP(MOCS ${imgsearch_MOCS})

add_executable ( imgsearch ${imgsearch_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( imgsearch  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
