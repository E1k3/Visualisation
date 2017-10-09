#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <string>
#include <vector>
#include <tuple>
#include <glm/glm.hpp>

#include "globject.h"


namespace vis
{
	/**
	 * @brief The Text class renders multiple lines of text at different locations.
	 * It uses freetype to render a font into a texture atlas at creation.
	 * Changing line locations is low overhead, changing text requires the upload of new data.
	 */
	class Text
	{
	public:
		explicit Text(unsigned height = 35, const std::string& font = "font.ttf");
		virtual ~Text() = default;

		void draw() const;

		/**
		 * @brief set_lines Sets text for each displayed line.
		 * @param lines The collection of lines to be displayed.
		 */
		void set_lines(const std::vector<std::string>& lines);
		/**
		 * @brief set_positions Sets the origin of each line.
		 * @param positions The collection of origin positions.
		 */
		void set_positions(const std::vector<glm::vec2>& positions);
		/**
		 * @brief set_viewport Sets the viewport size in pixel coordinates.
		 * @param viewport The viewport size.
		 */
		void set_viewport(const glm::ivec2& viewport);

		/**
		 * @brief total_relative_size Returns the size of all lines together, relative to the viewport size.
		 * @return The vec2 containing the size.
		 */
		glm::vec2 total_relative_size() const;
		/**
		 * @brief relative_sizes Returns the size for each line, relative to the viewport size.
		 * @return The vector containing a size vec2 for each line.
		 */
		std::vector<glm::vec2> relative_sizes() const;

	private:
		struct Glyph
		{
			glm::vec2 advance{0.f};
			glm::vec2 size{0.f};
			glm::vec2 bearing{0.f};
			float atlas_offset{0.f};
		};

		void update();

		int _atlas_width{0};
		int _atlas_height{0};

		std::vector<std::string> _vertex_shaders{"Shader/text_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"Shader/text_fs.glsl"};

		VertexArray _vao;
		Buffer _vbo;
		Texture _texture;
		Program _program;

		GLint _position_loc{-1};
		GLint _viewport_loc{-1};

		std::vector<GLint> _last_vertex_indices{};

		glm::ivec2 _viewport{1};
		std::vector<std::string> _lines{};
		std::vector<glm::vec2> _line_sizes{};
		std::vector<glm::vec2> _positions{};
		std::vector<std::tuple<std::string, glm::vec2>> _text{};

		std::vector<Glyph> _glyphs{};
	};
}

#endif // TEXTRENDERER_H
