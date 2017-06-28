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

		void draw(float delta_time);

	private:
		std::vector<float> genMask(unsigned width, unsigned height) const;

		EnsembleManager* _ensemble;
		InputManager* _input;

		unsigned _num_vertices;
		GLint _mvp_uniform;
		GLint _bounds_uniform;
		glm::vec3 _translate{0.f};
		float _scale{1.f};
	};
}

#endif // GLYPHRENDERER_H
