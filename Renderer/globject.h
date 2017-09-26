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
	 * @brief The GLObject class is a helper class that provides RAII style destruction of OpenGL objects.
	 * It losely followes the mechanics of std::unique_ptr.
	 */
	class GLObject
	{
	public:
		/**
		 * @brief GLObject<Type> Constructs a GL object from an OpenGL object ID
		 */
		explicit GLObject<Type>(GLuint id = 0);

		/**
		 * @brief GLObject Move constructor.
		 * Releases others object id and takes it.
		 */
		GLObject(GLObject<Type>&& other);
		/**
		 * @brief operator = Move assignment operator.
		 * Releases others object id and resets to it.
		 */
		GLObject<Type>& operator=(GLObject<Type>&& other);

		GLObject(const GLObject<Type>& other) = delete;
		GLObject<Type>& operator=(GLObject<Type>& other) = delete;

		/**
		 * @brief operator GLuint Enables implicit conversion of a GLObject to its GLuint id.
		 * This way, the object can be used in usual GL calls seamlessly.
		 */
		operator GLuint() const;

		/**
		 * @brief ~GLObject Destroys the object according to its type.
		 */
		virtual ~GLObject<Type>();

		/**
		 * @brief get Returns the id.
		 */
		GLuint get() const;
		/**
		 * @brief release Returns the id and gives up ownership of the object.
		 */
		GLuint release();
		/**
		 * @brief reset Destroys the currently held object and takes a new object by id.
		 */
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
