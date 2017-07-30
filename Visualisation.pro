TEMPLATE = app
CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lGL \
        -lGLEW \
        -lglfw \
        -lstdc++fs \
		-lfreetype

INCLUDEPATH += /usr/include/freetype2

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
    Data/field.cpp \
    Renderer/textrenderer.cpp \
    Renderer/colormaprenderer.cpp

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
    Data/field.h \
    Renderer/textrenderer.h \
    Renderer/colormaprenderer.h

DISTFILES += \
    Shader/heightfield_vs.glsl \
    Shader/heightfield_fs.glsl \
    Shader/glyph_vs.glsl \
    Shader/glyph_fs.glsl \
    Shader/gmm_glyph_vs.glsl \
    Shader/gmm_glyph_fs.glsl \
    Shader/gmm_heightfield_vs.glsl \
    Shader/gmm_heightfield_fs.glsl \
    Shader/gmm_glyph_fs_2.glsl \
    Shader/text_vs.glsl \
    Shader/text_fs.glsl \
    Shader/palette.glsl \
    Shader/glyph_gs.glsl \
    Shader/gmm_glyph_gs.glsl \
    Shader/colormap_fs.glsl \
    Shader/colormap_vs.glsl
