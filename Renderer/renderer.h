#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>

#include <GL/glew.h>

#include "globject.h"

namespace vis
{
	class Renderer
	{
	public:
		/**
		 * @brief load_shader Creates, loads and compiles shader from file.
		 * @param path The path at which the glsl file is located.
		 * @param type Shader type (one of the GLenum shadertypes defined by GLEW) <- bad practice
		 */
		static GLuint load_shader(std::string path, GLuint type);

		virtual void draw(float delta_time) = 0;

		GLuint gen_vao();

		GLuint gen_buffer();

		GLuint gen_texture();

		GLuint gen_program();

		static std::vector<float> gen_grid(int width, int height);

		static std::vector<unsigned> gen_grid_indices(int width, int height);

	private:

		/// GL IDs of the vertex array object.
		std::vector<GLObject> _vaos;
		/// GL IDs of vertex buffer objects.
		std::vector<GLObject> _buffers;
		/// GL IDs of textures.
		std::vector<GLObject> _textures;
		/// GL IDs of the shader program.
		std::vector<GLObject> _programs;
	};
}

#endif // RENDERER_H
