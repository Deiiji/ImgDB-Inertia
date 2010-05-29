TEMPLATE = lib
VERSION = 0.0.2
CONFIG += qt dll debug

TARGET = bin/lib/imgdb-dbg

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
