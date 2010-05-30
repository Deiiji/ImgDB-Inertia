TEMPLATE = lib
VERSION = 0.0.2
CONFIG += qt dll debug

OBJECTS_DIR = ../build/

TARGET = ../bin/lib/imgdb-dbg

DEFINES += DebugLib

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

#Check to see if LL provided qt libs are present (usually won't be)
#header files need to be installed

unix && exists( ../staticlibs/libllqtwebkit.a ) {
	LIBS += -L../staticlibs
}
