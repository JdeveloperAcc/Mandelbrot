QT += core gui network widgets

TARGET = WidgetApp

TEMPLATE = app

VERSION = 1.0.0.0

HEADERS = Widget.h MouseHoverEater.h RenderThread.h

SOURCES = main.cpp Widget.cpp MouseHoverEater.cpp RenderThread.cpp

CONFIG += debug

# install
target.path = ./WidgetApp
INSTALLS += target
