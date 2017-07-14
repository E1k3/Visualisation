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
    logger.cpp \
    Renderer/heightfieldrenderer.cpp \
    Renderer/renderer.cpp \
    application.cpp \
    inputmanager.cpp \
    Renderer/glyphrenderer.cpp \
    Data/math_util.cpp \
    Renderer/globject.cpp \
    Data/ensemble.cpp \
    Data/field.cpp

HEADERS += \
    logger.h \
    Renderer/heightfieldrenderer.h \
    Renderer/renderer.h \
    application.h \
    inputmanager.h \
    Renderer/glyphrenderer.h \
    Data/math_util.h \
    Renderer/globject.h \
    Data/ensemble.h \
    Data/field.h

DISTFILES += \
    Shader/heightfield_vs.glsl \
    Shader/heightfield_fs.glsl \
    Shader/glyph_vs.glsl \
    Shader/glyph_fs.glsl \
    Shader/gmm_glyph_vs.glsl \
    Shader/gmm_glyph_fs.glsl \
    Shader/gmm_heightfield_vs.glsl \
    Shader/gmm_heightfield_fs.glsl
