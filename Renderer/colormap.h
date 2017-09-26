#ifndef COLORMAPRENDERER_H
#define COLORMAPRENDERER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "text.h"
#include "primitives.h"

namespace vis
{
	/**
	 * @brief The Colormap class is capable of rendering a color map with division lines and labels.
	 */
	class Colormap
	{
	public:
		explicit Colormap();
		virtual ~Colormap() = default;

		/**
		 * @brief draw Draws the color map using the active OpenGL context.
		 */
		void draw() const;

		/**
		 * @brief set_position Sets the position of the lower left corner of the map in world coords.
		 */
		void set_position(const glm::vec2& position);

		/**
		 * @brief set_size Sets the size of the colormap in world coords.
		 */
		void set_size(const glm::vec2& size);

		/**
		 * @brief set_viewport Sets the viewport size in pixels that is used to calculate font size.
		 */
		void set_viewport(const glm::ivec2& viewport);
		/**
		 * @brief set_bounds Sets the lower and upper bound of the label values.
		 * Divisions will be created to get human readable label values and tries to stay close to the preferred amount.
		 */
		void set_bounds(const glm::vec2& bounds, int preferred_divisions);


	private:
		void update();

		Text _text{};
		Primitives _division_lines{{{0.f, 0.f, 0.f}, {0.f, 1.1f, 0.f}}};
		std::vector<glm::vec2> _text_positions{};

		glm::vec2 _position{-.75f, -1.f};
		glm::vec2 _size{1.5f, .05f};

		std::vector<float> _divisions{0.f, 1.f};
		glm::vec2 _bounds{0.f, 1.f};
		glm::ivec2 _viewport{15};

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
