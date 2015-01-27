#  This file is part of Cursed Earth.
#
#  Cursed Earth is an open source, cross-platform port of Evil Islands.
#  Copyright (C) 2009-2010 Yanis Kurganov.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program. If not, see <http://www.gnu.org/licenses/>.

AUTHOR = Yanis Kurganov
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
