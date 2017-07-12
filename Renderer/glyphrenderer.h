#ifndef GLYPHRENDERER_H
#define GLYPHRENDERER_H

#include <vector>

#include "renderer.h"
#include "inputmanager.h"
#include "Data/field.h"

namespace vis
{
	class GlyphRenderer : public Renderer
	{
	public:
		explicit GlyphRenderer(const std::vector<Field>& fields, InputManager& input);

		void init_gaussian(const std::vector<Field>& fields);
		void init_gmm(const std::vector<Field>& fields);
		virtual ~GlyphRenderer() = default;

		virtual void draw(float delta_time) override;

	private:
		std::vector<float> genMask(int width, int height) const;

		InputManager& _input;

		int _num_vertices;
		GLint _mvp_uniform;
		GLint _bounds_uniform;
		glm::vec3 _translate{0.f};
		float _scale{1.f};
	};
}

#endif // GLYPHRENDERER_H
