TEMPLATE = lib
VERSION = 0.0.2
CONFIG += qt dll debug

OBJECTS_DIR = ../build/

TARGET = ../bin/lib/imgdb-dbg

SUBDIRS += tests

SOURCES += bloom_filter.cpp \
	haar.cpp \ 
	imgdb.cpp

HEADERS += bloom_filter.h \
	haar.h\
	imgdb.h
	
unix || mac {
	DEFINES += LinuxBuild
}

unix:LIBS += -ljpeg -lpng -lz
