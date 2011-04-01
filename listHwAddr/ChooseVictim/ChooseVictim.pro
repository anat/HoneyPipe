#-------------------------------------------------
#
# Project created by QtCreator 2011-03-19T00:21:58
#
#-------------------------------------------------

QT       += core gui network

TARGET = ChooseVictim
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    arprequest.cpp \
    packet.cpp \
    rawsocket.cpp \
    netsoul.cpp \
    rsock.cpp

HEADERS  += mainwindow.h \
    arprequest.h \
    packet.h \
    rawsocket.h \
    IProtocol.hpp \
    netsoul.h \
    rsock.h

FORMS    += mainwindow.ui \
    netsoul.ui
