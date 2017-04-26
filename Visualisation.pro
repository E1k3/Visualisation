TEMPLATE = app
CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lGL \
        -lGLEW \
        -lglfw \
        -lstdc++fs

SOURCES += main.cpp \
    Data/timestep.cpp \
    logger.cpp \
    Data/ensemble.cpp

HEADERS += \
    Data/timestep.h \
    logger.h \
    Data/ensemble.h
