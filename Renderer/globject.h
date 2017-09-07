#ifndef GLOBJECT_H
#define GLOBJECT_H

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
	 * @brief The GLObject class TODO
	 */
	class GLObject
	{
	public:
		explicit GLObject<Type>(GLuint id = 0);

		GLObject(GLObject<Type>&& other);
		GLObject<Type>& operator=(GLObject<Type>&& other);

		GLObject(const GLObject<Type>& other) = delete;
		GLObject<Type>& operator=(GLObject<Type>& other) = delete;

		operator GLuint() const;

		virtual ~GLObject<Type>();

		GLuint get() const;

		GLuint release();

		void reset(GLuint id);

	private:
		void destroy();

		GLuint _id{0};
	};

	// Typedefs
	using VertexArray = GLObject<GLType::VERTEX_ARRAY>;
	using Buffer = GLObject<GLType::BUFFER>;
	using Texture = GLObject<GLType::TEXTURE>;
	using Program = GLObject<GLType::PROGRAM>;
	using Shader = GLObject<GLType::SHADER>;

	// GLObject implementation
	template<GLType Type> GLObject<Type>::GLObject(GLuint id) : _id{id}	{  }

	template<GLType Type> GLObject<Type>::GLObject(GLObject<Type>&& other) : _id{std::move(other.release())}	{ }

	template<GLType Type> GLObject<Type>& GLObject<Type>::operator=(GLObject<Type>&& other)
	{
		reset(other.release());
		return *this;
	}

	template<GLType Type> GLObject<Type>::operator GLuint() const	{ return _id; }

	template<GLType Type> GLObject<Type>::~GLObject()           	{ destroy(); }

	template<GLType Type> GLuint GLObject<Type>::get() const     	{ return _id; }

	template<GLType Type> GLuint GLObject<Type>::release()
	{
		auto id = _id;
		_id = 0;
		return id;
	}

	template<GLType Type> void GLObject<Type>::reset(GLuint id)
	{
		destroy();
		_id = id;
	}
}
#endif // GLOBJECT_H
