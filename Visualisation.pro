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
    glwindow.cpp \
    Data/timestep.cpp

HEADERS += \
    glwindow.h \
    Data/timestep.h
