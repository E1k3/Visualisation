#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <vector>
#include <glm/glm.hpp>

#include "globject.h"


namespace vis
{
	class Primitives
	{
	public:
		explicit Primitives(const std::vector<glm::vec3>& vertices);

		void update(const glm::mat4& mvp);

		void draw(GLenum mode = GL_LINES) const;

		void add_translation(const glm::vec3& translation);
		void set_translations(const std::vector<glm::vec3>& translations);

		void set_color(const glm::vec4& color);

	private:
		std::vector<std::string> _vertex_shaders{"/home/eike/Documents/Code/Visualisation/Shader/primitives_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"/home/eike/Documents/Code/Visualisation/Shader/primitives_fs.glsl"};
		int _vertex_count{0};

		glm::vec4 _color{1.f};

		// GL objects
		VertexArray _vao;
		Buffer _vbo;
		Program _program;

		// Transformations
		std::vector<float> _translations;

		// Uniforms
		GLint _mvp_loc{-1};
		GLint _translations_loc{-1};
		GLint _color_loc{-1};
	};
}

#endif // PRIMITIVES_H
