#ifndef GLYPHRENDERER_H
#define GLYPHRENDERER_H

#include <vector>

#include "renderer.h"
#include "Data/ensemblemanager.h"
#include "inputmanager.h"
#include "Data/field.h"

namespace vis
{
	class GlyphRenderer : public Renderer
	{
	public:
		explicit GlyphRenderer(const Timestep::ScalarField& mean_field, const Timestep::ScalarField& var_field, InputManager& input);
		explicit GlyphRenderer(const Timestep::ScalarField& mean_field, const Timestep::ScalarField& var_field, const Timestep::ScalarField& weight_field, InputManager& input);
		virtual ~GlyphRenderer() = default;

		virtual void draw(float delta_time) override;

	private:
		std::vector<float> genMask(unsigned width, unsigned height) const;

		InputManager& _input;

		unsigned _num_vertices;
		GLint _mvp_uniform;
		GLint _bounds_uniform;
		glm::vec3 _translate{0.f};
		float _scale{1.f};
	};
}

#endif // GLYPHRENDERER_H
