#include "glyphrenderer.h"

namespace vis
{
	GlyphRenderer::GlyphRenderer(EnsembleManager* ensemble, InputManager* input)
		: Renderer{},
		  _ensemble{ensemble},
		  _input{input}
	{

	}

	GlyphRenderer::GlyphRenderer(GlyphRenderer&& other) noexcept
		: GlyphRenderer{other._ensemble, other._input}
	{
		swap(*this, other);
	}

	GlyphRenderer& GlyphRenderer::operator=(GlyphRenderer other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	GlyphRenderer& GlyphRenderer::operator=(GlyphRenderer&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}
}
