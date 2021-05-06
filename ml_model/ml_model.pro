TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -lpthread -std=c++11
LIBS += -pthread

SOURCES += \
    ../CSVReader.cpp \
    ../dimension.cpp \
    ../globals.cpp \
    ../main.cpp \
    ../model.cpp \
    ../utils.cpp

DISTFILES += \
    ../README.md

HEADERS += \
    ../CSVReader.h \
    ../dimension.h \
    ../globals.h \
    ../model.h \
    ../structs.h \
    ../utils.h
