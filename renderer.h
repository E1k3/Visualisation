#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <string>

namespace vis
{
	class Renderer
	{
	public:
		/**
		 * @brief loadShader Creates, loads and compiles shader from file.
		 * @param path The path at which the glsl file is located.
		 * @param type Shader type (one of the GLenum shadertypes defined by GLEW) <- bad practice
		 */
		static unsigned loadShader(std::string path, unsigned type);

		unsigned genVao();

		unsigned genBuffer();

		unsigned genTexture();

		unsigned genProgram();

		static std::vector<float> genGrid(unsigned width, unsigned height);

		static std::vector<unsigned> genGridIndices(unsigned width, unsigned height);

		virtual ~Renderer();

	protected:
		/// @brief Renderer Default constructor.
		explicit Renderer() = default;
		/// @brief Renderer Default copy constructor.
		//explicit Renderer(const Renderer& other) = default;
		/// @brief Renderer Move constructor.
		explicit Renderer(Renderer&& other) noexcept;
		/// @brief operator= Copy assignment operator.
		Renderer& operator=(Renderer other) noexcept;

		/**
		 * @brief swap Swaps state of two HeightfieldRenderers.
		 */
		friend void swap(Renderer& first, Renderer& second) noexcept
		{
			using std::swap;
			swap(first._vaos, second._vaos);
			swap(first._buffers, second._buffers);
			swap(first._textures, second._textures);
			swap(first._programs, second._programs);
		}

	private:
		/// GL ID of the vertex array object.
		std::vector<unsigned> _vaos;
		/// GL IDs of vertex buffer objects.
		std::vector<unsigned> _buffers;
		/// GL IDs of textures.
		std::vector<unsigned> _textures;
		/// GL ID of the shader program.
		std::vector<unsigned> _programs;
	};
}

#endif // RENDERER_H
