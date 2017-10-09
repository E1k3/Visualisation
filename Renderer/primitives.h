#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <vector>
#include <glm/glm.hpp>

#include "globject.h"


namespace vis
{
	/**
	 * @brief The Primitives class renders a collection of vertices at different locations with different colors using instancing.
	 */
	class Primitives
	{
	public:
		explicit Primitives(const std::vector<glm::vec3>& vertices);

		void update(const glm::mat4& mvp);

		void draw(GLenum mode = GL_LINES) const;

		void add_translation(const glm::vec3& translation);
		void set_translations(const std::vector<glm::vec3>& translations);
		void clear_translations();

		void set_color(const glm::vec4& color);
		void add_color(const glm::vec4& color);
		void set_colors(const std::vector<glm::vec4>& colors);
		void clear_colors();

	private:
		std::vector<std::string> _vertex_shaders{"Shader/primitives_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"Shader/primitives_fs.glsl"};
		int _vertex_count{0};

		// GL objects
		VertexArray _vao;
		Buffer _vbo;
		Program _program;

		std::vector<float> _colors;
		std::vector<float> _translations;

		// Uniforms
		GLint _mvp_loc{-1};
		GLint _translations_loc{-1};
		GLint _colors_loc{-1};
	};
}

#endif // PRIMITIVES_H
