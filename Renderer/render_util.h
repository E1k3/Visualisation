#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <vector>
#include <tuple>
#include <GL/glew.h>


namespace vis
{
	namespace render_util
	{
		void load_compile_shader(GLuint id, const std::vector<std::string>& paths);

		std::vector<float> gen_grid(int width, int height);

		std::vector<GLuint> gen_grid_indices(int width, int height);
	}
}

#endif // RENDER_UTIL_H
