#ifndef LINES_H
#define LINES_H

#include <vector>
#include <glm/glm.hpp>

#include "globject2.h"


namespace vis
{
	class Lines
	{
	public:
		explicit Lines(const std::vector<glm::vec3>& vertices);

		void update(const glm::mat4& mvp);

		void draw() const;

		void add_translation(const glm::vec3& translation);
		void set_translations(const std::vector<glm::vec3>& translations);

	private:
		std::vector<std::string> _vertex_shaders{"/home/eike/Documents/Code/Visualisation/Shader/lines_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"/home/eike/Documents/Code/Visualisation/Shader/lines_fs.glsl"};
		int _vertex_count{0};

		// GL objects
		VertexArray _vao;
		Buffer _vbo;
		Program _program;

		// Transformations
		std::vector<float> _translations;

		// Uniforms
		GLint _mvp_loc{-1};
		GLint _translations_loc{-1};
	};
}

#endif // LINES_H
