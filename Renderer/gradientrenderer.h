#ifndef GRADIENTRENDERER_H
#define GRADIENTRENDERER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "renderer.h"
#include "textrenderer.h"

namespace vis
{
	class GradientRenderer : public Renderer
	{
	public:
		explicit GradientRenderer();
		virtual ~GradientRenderer() = default;

		void set_position(const glm::vec2& position);
		void set_size(const glm::vec2& size);
		void set_viewport(const glm::ivec2& viewport);
		void set_bounds(const glm::vec2& bounds);

		void draw(float delta_time, float total_time) override;

	private:
		TextRenderer _text{};

		glm::vec2 _bounds{0.f};
		glm::ivec2 _viewport{1};

		glm::vec2 _position{};
		glm::vec2 _size{};

		GLint _position_uniform{0};
		GLint _size_uniform{0};

		GLuint _vao{0};
		GLuint _program{0};
	};
}
#endif // GRADIENTRENDERER_H
