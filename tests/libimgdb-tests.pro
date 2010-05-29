TEMPLATE = app
VERSION = 0.0.2
CONFIG += qt debug

OBJECTS_DIR = ../build/

TARGET = ../bin/imgdb-test

DEPENDPATH += ../ImgDB
INCLUDEPATH += ../ImgDB

SOURCES += test.cpp

HEADERS += bloom_filter.h \
	haar.h\
	imgdb.h
	
unix || mac {
	DEFINES += LinuxBuild
}

unix:LIBS += -ljpeg -lpng -lz -L ../bin/lib -limgdb-dbg
