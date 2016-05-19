QT += core
QT -= gui

TARGET = otfwriter
CONFIG += console
CONFIG -= app_bundle

LIBS = -L/usr/lib \
       -lm \
       -lotf

TEMPLATE = app

SOURCES += main.cpp

