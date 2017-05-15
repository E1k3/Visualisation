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

		int _mvp_uniform;
		glm::vec3 _cam_position{glm::vec3{0.f, 0.f, 1.8f}};
	};
}

#endif // GLYPHRENDERER_H
