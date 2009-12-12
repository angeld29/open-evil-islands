AUTHOR = Yanis V. Kurganov
EMAIL = ykurganov@users.sourceforge.net

TEMPLATE = app
LANGUAGE = C++
LICENSE = GPL

CONFIG += qt no_keywords
QT += core gui

win32:debug:CONFIG += console

TARGET = resfileviewer
DESTDIR = bin

unix {
	debug:OBJECTS_DIR = build/obj/posix/debug
	release:OBJECTS_DIR = build/obj/posix/release
}

mac {
	debug:OBJECTS_DIR = build/obj/darwin/debug
	release:OBJECTS_DIR = build/obj/darwin/release
}

win32 {
	debug:OBJECTS_DIR = build/obj/win32/debug
	release:OBJECTS_DIR = build/obj/win32/release
}

MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/uic

LIBS += -L../cecore/lib -lcecore
QMAKE_LFLAGS += -Wl,-rpath,../../cecore/lib

INCLUDEPATH += src ../cecore/include

FORMS = forms/mainwindow.ui

HEADERS = src/gui/treewidget.hpp \
			src/gui/mainwindow.hpp

SOURCES = src/gui/treewidget.cpp \
			src/gui/mainwindow.cpp \
			src/main.cpp
