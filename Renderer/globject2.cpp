#include "globject2.h"
#include <memory>

namespace vis
{
	template<> void GLObject2<GLType::VERTEX_ARRAY>::destroy()	{ glDeleteVertexArrays(1, &_id); }
	template<> void GLObject2<GLType::BUFFER>::destroy()      	{ glDeleteBuffers(1, &_id); }
	template<> void GLObject2<GLType::TEXTURE>::destroy()     	{ glDeleteTextures(1, &_id); }
	template<> void GLObject2<GLType::SHADER>::destroy()     	{ glDeleteShader(_id); }
	template<> void GLObject2<GLType::PROGRAM>::destroy()     	{ glDeleteProgram(_id); }

	VertexArray gen_vertex_array()
	{
		GLuint id = 0;
		glGenVertexArrays(1, &id);
		return VertexArray(id);
	}

	Buffer gen_buffer()
	{
		GLuint id = 0;
		glGenBuffers(1, &id);
		return Buffer(id);
	}

	Texture gen_texture()
	{
		GLuint id = 0;
		glGenTextures(1, &id);
		return Texture(id);
	}

	Program gen_program()
	{
		return Program{glCreateProgram()};
	}

	Shader gen_shader(GLenum shadertype)
	{
		return Shader(glCreateShader(shadertype));
	}
}
