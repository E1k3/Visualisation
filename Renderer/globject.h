#ifndef GLOBJECT_H
#define GLOBJECT_H

#include <functional>
#include <exception>

/**
 * @brief The GLObject class wraps a single OpenGL object, to manage its ownership/lifetime.
 */
class GLObject
{
public:
	/**
	 * @brief GLObject Constructs a GLObject with its id and deleter function.
	 * @param id The OpenGL ID.
	 * @param deleter The function that has to be called for OpenGL to delete the object.
	 */
	GLObject(unsigned id, std::function<void(unsigned)> deleter);
	/// @brief GLObject No copy construction.
	GLObject(const GLObject& other) = delete;
	/**
	 * @brief GLObject Deletes currently wrapped OpenGL object and replaces it with another one.
	 */
	GLObject(GLObject&& other);
	/// @brief operator= No copy assignment.
	GLObject& operator=(const GLObject& other) = delete;
	/**
	 * @brief operator = Deletes currently wrapped OpenGL object and replaces it with another one.
	 */
	GLObject& operator=(GLObject&& other);
	/**
	 * @brief ~GLObject Deletes the wrapped OpenGL object using the deleter function.
	 */
	~GLObject();

	/**
	 * @brief release Returns the id of the wrapped object and releases the ownership.
	 * @return The OpenGL ID.
	 */
	unsigned release();

	/**
	 * @brief reset Replaces the wrapped object, keeping the deleter.
	 * @param id The new OpenGL ID.
	 */
	void reset(unsigned id);
	/**
	 * @brief reset Replaces the wrapped object.
	 * @param id The new OpenGL ID.
	 * @param deleter The new deleter function.
	 */
	void reset(unsigned id, std::function<void(unsigned)> deleter);
	/**
	 * @brief get Returns the OpenGL ID of the currently wrapped object.
	 */
	unsigned get() const;

private:
	unsigned _id{0};
	std::function<void(unsigned)> _deleter{ [] (unsigned) { throw std::runtime_error("GLObject with empty deleter was destroyed."); } };
};

#endif // GLOBJECT_H
