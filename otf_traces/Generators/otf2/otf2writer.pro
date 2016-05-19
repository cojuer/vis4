TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

LIBS = -L/usr/lib \
    -lotf2 \
    -L/opt/otf2/lib \
    -Wl,-rpath=/opt/otf2/lib

INCLUDEPATH += /opt/otf2/include
