#ifndef COLORMAPRENDERER_H
#define COLORMAPRENDERER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "renderer.h"
#include "textrenderer.h"

namespace vis
{
	class ColormapRenderer : public Renderer
	{
	public:
		explicit ColormapRenderer();
		virtual ~ColormapRenderer() = default;

		void set_position(const glm::vec2& position);
		void set_size(const glm::vec2& size);
		void set_viewport(const glm::ivec2& viewport);
		void set_bounds(const glm::vec2& bounds);
		void set_divisions(int divisions);

		void draw(float delta_time, float total_time) override;

	private:
		void update();

		TextRenderer _text{};
		std::vector<std::string> _text_lines{};
		std::vector<glm::vec2> _text_positions{};

		glm::vec2 _position{0.f};
		glm::vec2 _size{1.f};
		glm::ivec2 _viewport{1};
		glm::vec2 _bounds{0.f, 1.f};
		int _divisions{5};

		GLint _position_uniform{0};
		GLint _size_uniform{0};
		GLint _viewport_uniform{0};
		GLint _division_uniform{0};

		GLuint _vao{0};
		GLuint _program{0};
	};
}
#endif // COLORMAPRENDERER_H
