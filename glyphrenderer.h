#ifndef GLYPHRENDERER_H
#define GLYPHRENDERER_H

#include "renderer.h"
#include "Data/ensemblemanager.h"
#include "inputmanager.h"

namespace vis
{
	class GlyphRenderer : public Renderer
	{
	public:
		explicit GlyphRenderer(EnsembleManager* ensemble, InputManager* input);

		explicit GlyphRenderer(const GlyphRenderer& other) = default;

		explicit GlyphRenderer(GlyphRenderer&& other) noexcept;

		GlyphRenderer& operator=(GlyphRenderer other) noexcept;

		GlyphRenderer& operator=(GlyphRenderer&& other) noexcept;

		friend void swap(GlyphRenderer& first, GlyphRenderer& second)
		{
			using std::swap;
			swap(static_cast<Renderer&>(first), static_cast<Renderer&>(second));

			swap(first._ensemble, second._ensemble);
			swap(first._input, second._input);
		}

	private:
		EnsembleManager* _ensemble;
		InputManager* _input;


	};
}

#endif // GLYPHRENDERER_H
