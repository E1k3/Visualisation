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
		 * @brief loadShader Creates, loads and compiles shader from file.
		 * @param path The path at which the glsl file is located.
		 * @param type Shader type (one of the GLenum shadertypes defined by GLEW) <- bad practice
		 */
		static unsigned loadShader(std::string path, GLuint type);

		GLuint genVao();

		GLuint genBuffer();

		GLuint genTexture();

		GLuint genProgram();

		static std::vector<float> genGrid(unsigned width, unsigned height);

		static std::vector<unsigned> genGridIndices(unsigned width, unsigned height);

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
