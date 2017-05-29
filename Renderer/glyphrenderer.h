#ifndef GLYPHRENDERER_H
#define GLYPHRENDERER_H

#include <vector>

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

		void draw(float delta_time);

	private:
		std::vector<float> genMask(unsigned width, unsigned height) const;

		EnsembleManager* _ensemble;
		InputManager* _input;

		unsigned _num_vertices;
		int _mvp_uniform;
		int _bounds_uniform;
		glm::vec3 _translate{0.f};
		float _scale{1.f};
	};
}

#endif // GLYPHRENDERER_H
