cmake_minimum_required( VERSION 2.6 )

set(ROOT_PROJECT_NAME "ImgDB-Inertia" CACHE STRING
    "The root project/makefile/solution name.")
project(${ROOT_PROJECT_NAME})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Platform)
include(00-Common)

set ( CMAKE_BUILD_TYPE Debug )

add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	)

add_subdirectory( ImgDB )
add_subdirectory( tests )
add_subdirectory( imgsearch )
