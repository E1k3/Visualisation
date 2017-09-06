#ifndef COLORMAPRENDERER_H
#define COLORMAPRENDERER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "text.h"
#include "primitives.h"

namespace vis
{
	class Colormap
	{
	public:
		explicit Colormap();
		virtual ~Colormap() = default;

		void draw() const;

		void set_position(const glm::vec2& position);

		void set_size(const glm::vec2& size);

		void set_viewport(const glm::ivec2& viewport);

		void set_bounds(const glm::vec2& bounds, int preferred_divisions);


	private:
		void update();

		Text _text{};
		Primitives _division_lines{{{0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}}};
		std::vector<glm::vec2> _text_positions{};

		glm::vec2 _position{-.75f, -1.f};
		glm::vec2 _size{1.5f, .05f};

		std::vector<float> _divisions{0.f, 1.f};
		glm::vec2 _bounds{0.f, 1.f};

		GLint _position_loc{-1};
		GLint _size_loc{-1};

		std::vector<std::string> _vertex_shaders{"/home/eike/Documents/Code/Visualisation/Shader/colormap_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"/home/eike/Documents/Code/Visualisation/Shader/colormap_fs.glsl",
												   "/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"};
		VertexArray _vao;
		Buffer _vbo;
		Program _program;
	};
}
#endif // COLORMAPRENDERER_H
