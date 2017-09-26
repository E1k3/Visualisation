#include "globject.h"

namespace vis
{
	template<> void GLObject<GLType::VERTEX_ARRAY>::destroy()	{ glDeleteVertexArrays(1, &_id); }
	template<> void GLObject<GLType::BUFFER>::destroy()      	{ glDeleteBuffers(1, &_id); }
	template<> void GLObject<GLType::TEXTURE>::destroy()     	{ glDeleteTextures(1, &_id); }
	template<> void GLObject<GLType::SHADER>::destroy()     	{ glDeleteShader(_id); }
	template<> void GLObject<GLType::PROGRAM>::destroy()     	{ glDeleteProgram(_id); }
}
