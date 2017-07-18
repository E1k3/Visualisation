#include "renderer.h"

#include "logger.h"

#include <fstream>
#include <streambuf>
#include <algorithm>

namespace vis
{
	GLuint Renderer::load_shader(std::string path, GLuint type)
	{
		// Check requirements
		auto types = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
					  GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
					  GL_GEOMETRY_SHADER};
		if(std::find(types.begin(), types.end(), type) == types.end())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Shader type is invalid.";
			// TODO:ERROR handling. Shadertype is invalid.
			throw std::runtime_error("Load Shader Error");
		}
		// Load code
		auto ifs = std::ifstream(path);
		auto code = std::vector<char>{};
		std::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(),
				  std::back_inserter(code));
		code.push_back('\0');
		const char* code_ptr = &code[0];

		// Create, compile
		auto id = glCreateShader(type);
		glShaderSource(id, 1, &code_ptr, nullptr);
		glCompileShader(id);

		// Check for errors
		int success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if(success != GL_TRUE)
		{
			int logsize = 0;
			std::vector<char> log;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logsize);
			log.resize(static_cast<unsigned>(logsize+1));
			glGetShaderInfoLog(id, logsize+1, nullptr, &log[0]);

			Logger::instance() << Logger::Severity::ERROR
							   << "Shader did not compile."
							   << &log[0];
			// TODO:ERROR handling. Shader compilation failed.
			throw std::runtime_error("Load Shader Error");
		}

		return id;
	}

	GLuint Renderer::gen_vao()
	{
		constexpr auto deleter = [] (GLuint id) { glDeleteVertexArrays(1, &id); };
		GLuint id = 0;
		glGenVertexArrays(1, &id);
		_vaos.push_back(GLObject(id, deleter));
		return _vaos.back().get();
	}

	GLuint Renderer::gen_buffer()
	{
		constexpr auto deleter = [] (GLuint id) { glDeleteBuffers(1, &id); };
		GLuint id = 0;
		glGenBuffers(1, &id);
		_buffers.push_back(GLObject(id, deleter));
		return _buffers.back().get();
	}

	GLuint Renderer::gen_texture()
	{
		constexpr auto deleter = [] (GLuint id) { glDeleteTextures(1, &id); };
		GLuint id = 0;
		glGenTextures(1, &id);
		_textures.push_back(GLObject(id, deleter));
		return _textures.back().get();
	}

	GLuint Renderer::gen_program()
	{
		constexpr auto deleter = [] (GLuint id) { glDeleteProgram(id); };
		_programs.push_back(GLObject(glCreateProgram(), deleter));
		return _programs.back().get();
	}

	std::vector<float> Renderer::gen_grid_indexed(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Grid generation using negative dimensions.\n"
							   << "width: " << width << " height: " << height;
			throw std::invalid_argument("Negative grid generation dimensions");
		}


		auto grid = std::vector<float>(static_cast<size_t>(width * height * 4));

		if(width*height == 0)
			return grid;	// TODO:error handling?

		float uv_y = 1.f;
		for(int row = 0; row < height; ++row)
		{
			float uv_x = 0.f;
			for(int col = 0; col < width; ++col)
			{
				float x = col/(width-1.f);
				float y = row/(height-1.f);
				grid[static_cast<size_t>(row*width + col)*4] = x*2.f - 1.f;
				grid[static_cast<size_t>(row*width + col)*4 + 1] = y*2.f - 1.f;
				grid[static_cast<size_t>(row*width + col)*4 + 2] = uv_x;
				grid[static_cast<size_t>(row*width + col)*4 + 3] = uv_y;

				uv_x = 1.f - uv_x;
			}
			uv_y = 1.f - uv_y;
		}
		return grid;
	}

	std::vector<float> Renderer::gen_grid(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Grid generation using negative dimensions.\n"
							   << "width: " << width << " height: " << height;
			throw std::invalid_argument("Negative grid generation dimensions");
		}

		// !!!TOOODOO:IMPLEMENT THIS WITHOUT INDEXING!!!PRIO!!!
		auto grid = std::vector<float>(static_cast<size_t>(width * height * 4));

		if(width*height == 0)
			return grid;	// TODO:error handling?

		float uv_y = 1.f;
		for(int row = 0; row < height; ++row)
		{
			float uv_x = 0.f;
			for(int col = 0; col < width; ++col)
			{
				float x = col/(width-1.f);
				float y = row/(height-1.f);
				grid[static_cast<size_t>(row*width + col)*4] = x*2.f - 1.f;
				grid[static_cast<size_t>(row*width + col)*4 + 1] = y*2.f - 1.f;
				grid[static_cast<size_t>(row*width + col)*4 + 2] = uv_x;
				grid[static_cast<size_t>(row*width + col)*4 + 3] = uv_y;

				uv_x = 1.f - uv_x;
			}
			uv_y = 1.f - uv_y;
		}
		return grid;
	}

	std::vector<unsigned> Renderer::gen_grid_indices(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Grid index generation using negative dimensions.\n"
							   << "width: " << width << " height: " << height;
			throw std::invalid_argument("Negative grid index generation dimensions");
		}

		auto indices = std::vector<unsigned>{};
		if(width*height <= 2)
			return indices;	// TODO:error handling?

		for(int y = 0; y < height-1; ++y)
		{
			for(int x = 0; x < width-1; ++x)
			{
				unsigned i = static_cast<unsigned>(y*width + x);
				indices.push_back(i+static_cast<unsigned>(width));
				indices.push_back(i+static_cast<unsigned>(width)+1);
				indices.push_back(i);
				indices.push_back(i+static_cast<unsigned>(width)+1);
				indices.push_back(i+1);
				indices.push_back(i);
			}
		}
		return indices;
	}
}
