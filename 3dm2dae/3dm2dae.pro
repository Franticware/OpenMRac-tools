TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -Wno-stringop-truncation

SOURCES += \
        3dm.cpp \
        main.cpp

HEADERS += \
    3dm.h \
    cstr1.h
