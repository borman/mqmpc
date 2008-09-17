TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= libmpdclient.h \
		  mainwidget.h \
		  ui_general.h
SOURCES		= libmpdclient.c \
		  main.cpp \
		  mainwidget.cpp \
		  ui_general.cpp
INTERFACES	=
TMAKE_CFLAGS += -DEZX
TMAKE_CXXFLAGS += -DEZX
