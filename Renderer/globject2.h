#ifndef GLOBJECT2_H
#define GLOBJECT2_H

#include <functional>
#include <GL/glew.h>

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

		template<typename = std::enable_if<Type == GLType::SHADER>>
		explicit GLObject2<Type>(GLenum shadertype, GLuint id);


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

	using VertexArray = GLObject2<GLType::VERTEX_ARRAY>;
	using Buffer = GLObject2<GLType::BUFFER>;
	using Texture = GLObject2<GLType::TEXTURE>;
	using Shader = GLObject2<GLType::SHADER>;
	using Program = GLObject2<GLType::PROGRAM>;

	template<> GLObject2<GLType::VERTEX_ARRAY>::GLObject2()	{ if(_id == 0) glGenVertexArrays(1, &_id); }
	template<> GLObject2<GLType::BUFFER>::GLObject2()      	{ if(_id == 0) glGenBuffers(1, &_id); }
	template<> GLObject2<GLType::TEXTURE>::GLObject2()     	{ if(_id == 0) glGenTextures(1, &_id); }
	template<> GLObject2<GLType::PROGRAM>::GLObject2()     	{ if(_id == 0) _id = glCreateProgram(); }
	template<> GLObject2<GLType::SHADER>::GLObject2()      	{ Logger::error() << "A shader without type or id was created."; throw std::invalid_argument{"Shader without type created"}; }

	template<GLType Type> GLObject2<Type>::GLObject2(GLuint id) : _id{id}	{  }
	// Define special (GLenum) ctor for shaders, results in compile error when GLenum != GLuint
	template<> GLObject2<GLType::SHADER>::GLObject2(GLenum shadertype)	{ _id = glCreateShader(shadertype); }

	template<> template<> GLObject2<GLType::SHADER>::GLObject2(GLenum shadertype, GLuint id) : _id{id}	{ if(_id == 0) _id = glCreateShader(shadertype); }

	template<GLType Type> GLObject2<Type>::GLObject2(GLObject2<Type>&& other) : _id{other.release()}	{ }

	template<GLType Type> GLObject2<Type>& GLObject2<Type>::operator=(GLObject2<Type>&& other)
	{
		reset(other.release());
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

	template<> void GLObject2<GLType::VERTEX_ARRAY>::destroy()	{ glDeleteVertexArrays(1, &_id); }
	template<> void GLObject2<GLType::BUFFER>::destroy()      	{ glDeleteBuffers(1, &_id); }
	template<> void GLObject2<GLType::TEXTURE>::destroy()     	{ glDeleteTextures(1, &_id); }
	template<> void GLObject2<GLType::SHADER>::destroy()     	{ glDeleteShader(_id); }
	template<> void GLObject2<GLType::PROGRAM>::destroy()     	{ glDeleteProgram(_id); }
}
#endif // GLOBJECT2_H
