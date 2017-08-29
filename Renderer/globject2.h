#ifndef GLOBJECT2_H
#define GLOBJECT2_H

#include "GL/glew.h"

#include "logger.h"

namespace vis
{
	enum class GLType
	{
		VERTEX_ARRAY = 0,
		BUFFER,
		TEXTURE,
		SHADER,
		PROGRAM,
	};

	template<GLType Type>
	class GLObject2
	{
	public:
		explicit GLObject2<Type>();
		explicit GLObject2<Type>(GLuint id);

		GLObject2(const GLObject2<Type>& other) = delete;
		explicit GLObject2(GLObject2<Type>&& other);

		GLObject2<Type>& operator=(GLObject2<Type>& other) = delete;
		GLObject2<Type>& operator=(GLObject2<Type>&& other);

		virtual ~GLObject2<Type>();

		GLuint get() const;

		GLuint release();

		void reset(GLuint id);

	private:
		void destroy();

		GLuint _id{0};
	};

	using Vao = GLObject2<GLType::VERTEX_ARRAY>;
	using Vbo = GLObject2<GLType::BUFFER>;
	using Texture = GLObject2<GLType::TEXTURE>;
	using Shader = GLObject2<GLType::SHADER>;
	using Program = GLObject2<GLType::PROGRAM>;

	template<GLType Type> GLObject2<Type>::GLObject2()		{ Logger::warning() << "GLObject with not handled type was created."; }
	template<> GLObject2<GLType::VERTEX_ARRAY>::GLObject2()	{ glGenVertexArrays(1, &_id); }
	template<> GLObject2<GLType::BUFFER>::GLObject2()      	{ glGenBuffers(1, &_id); }
	template<> GLObject2<GLType::TEXTURE>::GLObject2()     	{ glGenTextures(1, &_id); }
	template<> GLObject2<GLType::PROGRAM>::GLObject2()     	{ _id = glCreateProgram(); }

	template<GLType Type> GLObject2<Type>::GLObject2(GLuint id) : _id{id}	{ }

	template<GLType Type> GLObject2<Type>::GLObject2(GLObject2<Type>&& other) : _id{other.release()}	{ }

	template<GLType Type> GLObject2<Type>& GLObject2<Type>::operator=(GLObject2<Type>&& other)
	{
		_id = other.release();
		return *this;
	}

	template<GLType Type> GLObject2<Type>::~GLObject2()    	{ destroy(); }

	template<GLType Type> GLuint GLObject2<Type>::get() const	{ return _id; }

	template<GLType Type> GLuint GLObject2<Type>::release()
	{
		auto id = _id;
		_id = 0;
		return _id;
	}

	template<GLType Type> void GLObject2<Type>::reset(GLuint id)
	{
		destroy();
		_id = id;
	}

	template<GLType Type> void GLObject2<Type>::destroy()     	{ Logger::warning() << "GLObject with not handled type was destroyed."; }
	template<> void GLObject2<GLType::VERTEX_ARRAY>::destroy()	{ glDeleteVertexArrays(1, &_id); }
	template<> void GLObject2<GLType::BUFFER>::destroy()      	{ glDeleteBuffers(1, &_id); }
	template<> void GLObject2<GLType::TEXTURE>::destroy()     	{ glDeleteTextures(1, &_id); }
	template<> void GLObject2<GLType::SHADER>::destroy()     	{ glDeleteShader(_id); }
	template<> void GLObject2<GLType::PROGRAM>::destroy()     	{ glDeleteProgram(_id); }
}
#endif // GLOBJECT2_H
