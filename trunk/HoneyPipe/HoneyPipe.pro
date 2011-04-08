#-------------------------------------------------
#
# Project created by QtCreator 2011-03-19T00:21:58
#
#-------------------------------------------------

QT       += core gui network webkit

TARGET = HoneyPipe
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    arprequest.cpp \
    packet.cpp \
    rawsocket.cpp \
    netsoul.cpp \
    rsock.cpp \
    http.cpp \
    changemessage.cpp

HEADERS  += mainwindow.h \
    arprequest.h \
    packet.h \
    rawsocket.h \
    IProtocol.hpp \
    netsoul.h \
    rsock.h \
    http.h \
    changemessage.h

FORMS    += mainwindow.ui \
    netsoul.ui \
    http.ui \
    changemessage.ui
