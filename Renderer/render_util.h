#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <vector>
#include <tuple>
#include <GL/glew.h>

#include "globject.h"


namespace vis
{
	/**
	 * General utility class for renderer or OpenGL related problems.
	 */
	namespace render_util
	{
		void load_compile_shader(GLuint id, const std::vector<std::string>& paths);

		std::vector<float> gen_grid(int width, int height);

		std::vector<GLuint> gen_grid_indices(int width, int height);

		/**
		 * @brief get_uniform_colormap_texture Singleton-like access to a CET perceptually uniform 1D colormap texture.
		 */
		Texture& get_uniform_colormap_texture();

		Texture create_colormap_texture(const std::vector<float>& data);

		VertexArray gen_vertex_array();
		Buffer gen_buffer();
		Texture gen_texture();
		Program gen_program();
		Shader gen_shader(GLenum shadertype);
	}
}

#endif // RENDER_UTIL_H
