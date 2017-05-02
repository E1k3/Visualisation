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
    Data/ensemblemanager.cpp \
    heightfieldrenderer.cpp \
    renderer.cpp

HEADERS += \
    Data/timestep.h \
    logger.h \
    Data/ensemblemanager.h \
    heightfieldrenderer.h \
    renderer.h

DISTFILES += \
    Shader/heightfield_vs.glsl \
    Shader/heightfield_fs.glsl
