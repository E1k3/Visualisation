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
    Renderer/heightfieldrenderer.cpp \
    Renderer/renderer.cpp \
    application.cpp \
    inputmanager.cpp \
    Renderer/glyphrenderer.cpp \
    Data/math_util.cpp \
    Renderer/globject.cpp

HEADERS += \
    Data/timestep.h \
    logger.h \
    Data/ensemblemanager.h \
    Renderer/heightfieldrenderer.h \
    Renderer/renderer.h \
    application.h \
    inputmanager.h \
    Renderer/glyphrenderer.h \
    Data/math_util.h \
    Renderer/globject.h

DISTFILES += \
    Shader/heightfield_vs.glsl \
    Shader/heightfield_fs.glsl \
    Shader/glyph_vs.glsl \
    Shader/glyph_fs.glsl
