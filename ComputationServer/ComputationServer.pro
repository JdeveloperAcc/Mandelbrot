QT += core network

TARGET = ComputationServer

TEMPLATE = app

VERSION = 1.0.0.0

HEADERS = Server.h RenderThread.h HttpProtocol.h

SOURCES = main.cpp Server.cpp RenderThread.cpp HttpProtocol.cpp

CONFIG += debug

# install
target.path = ./ComputationServer
INSTALLS += target
