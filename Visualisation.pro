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
    application.cpp \
    inputmanager.cpp \
    Data/math_util.cpp \
    Data/ensemble.cpp \
    Data/field.cpp \
    Renderer/glyph.cpp \
    Renderer/render_util.cpp \
    Renderer/glyphgmm.cpp \
    Renderer/heightfield.cpp \
    Renderer/heightfieldgmm.cpp \
    Renderer/primitives.cpp \
    Renderer/text.cpp \
    Renderer/colormap.cpp \
    Renderer/globject.cpp \
    Renderer/visualization.cpp

HEADERS += \
    logger.h \
    application.h \
    inputmanager.h \
    Data/math_util.h \
    Data/ensemble.h \
    Data/field.h \
    Renderer/glyph.h \
    Renderer/render_util.h \
    Renderer/glyphgmm.h \
    Renderer/heightfield.h \
    Renderer/heightfieldgmm.h \
    Renderer/primitives.h \
    Renderer/text.h \
    Renderer/colormap.h \
    Renderer/globject.h \
    Renderer/visualization.h

DISTFILES += \
    Shader/heightfield_vs.glsl \
    Shader/heightfield_fs.glsl \
    Shader/glyph_vs.glsl \
    Shader/glyph_fs.glsl \
    Shader/gmm_glyph_vs.glsl \
    Shader/gmm_heightfield_vs.glsl \
    Shader/gmm_heightfield_fs.glsl \
    Shader/text_vs.glsl \
    Shader/text_fs.glsl \
    Shader/palette.glsl \
    Shader/glyph_gs.glsl \
    Shader/gmm_glyph_gs.glsl \
    Shader/colormap_fs.glsl \
    Shader/colormap_vs.glsl \
    Shader/primitives_fs.glsl \
    Shader/primitives_vs.glsl \
    Shader/gmm_glyph_fs_alt.glsl \
    Shader/gmm_glyph_fs.glsl
