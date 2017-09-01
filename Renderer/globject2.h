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
	/**
	 * @brief The GLObject2 class TODO
	 */
	class GLObject2
	{
	public:
		explicit GLObject2<Type>(GLuint id = 0);

		GLObject2(GLObject2<Type>&& other);
		GLObject2<Type>& operator=(GLObject2<Type>&& other);

		GLObject2(const GLObject2<Type>& other) = delete;
		GLObject2<Type>& operator=(GLObject2<Type>& other) = delete;

		operator GLuint() const;

		virtual ~GLObject2<Type>();

		GLuint get() const;

		GLuint release();

		void reset(GLuint id);

	private:
		void destroy();

		GLuint _id{0};
	};

	// Typedefs
	using VertexArray = GLObject2<GLType::VERTEX_ARRAY>;
	using Buffer = GLObject2<GLType::BUFFER>;
	using Texture = GLObject2<GLType::TEXTURE>;
	using Program = GLObject2<GLType::PROGRAM>;
	using Shader = GLObject2<GLType::SHADER>;

	// Helper functions
	VertexArray gen_vertex_array();
	Buffer gen_buffer();
	Texture gen_texture();
	Program gen_program();
	Shader gen_shader(GLenum shadertype);

	// GLObject implementation
	template<GLType Type> GLObject2<Type>::GLObject2(GLuint id) : _id{id}	{  }

	template<GLType Type> GLObject2<Type>::GLObject2(GLObject2<Type>&& other) : _id{std::move(other.release())}	{ }

	template<GLType Type> GLObject2<Type>& GLObject2<Type>::operator=(GLObject2<Type>&& other)
	{
		reset(other.release());
		return *this;
	}

	template<GLType Type> GLObject2<Type>::operator GLuint() const	{ return _id; }

	template<GLType Type> GLObject2<Type>::~GLObject2()           	{ destroy(); }

	template<GLType Type> GLuint GLObject2<Type>::get() const     	{ return _id; }

	template<GLType Type> GLuint GLObject2<Type>::release()
	{
		auto id = _id;
		_id = 0;
		return id;
	}

	template<GLType Type> void GLObject2<Type>::reset(GLuint id)
	{
		destroy();
		_id = id;
	}
}
#endif // GLOBJECT2_H
