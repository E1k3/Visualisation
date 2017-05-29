#include "renderer.h"

#include "logger.h"

#include <fstream>
#include <streambuf>
#include <algorithm>

#include <GL/glew.h>

namespace vis
{
	unsigned Renderer::loadShader(std::string path, unsigned type)
	{
		// Check requirements
		auto types = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER,
					  GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER,
					  GL_GEOMETRY_SHADER};
		if(std::find(types.begin(), types.end(), type) == types.end())
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Shader type is invalid." << std::endl;
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
							   << &log[0]
							   << std::endl;
			// TODO:ERROR handling. Shader compilation failed.
			throw std::runtime_error("Load Shader Error");
		}

		return id;
	}

	unsigned Renderer::genVao()
	{
		_vaos.push_back(0);
		glGenVertexArrays(1, &_vaos.back());
		return _vaos.back();
	}

	unsigned Renderer::genBuffer()
	{
		_buffers.push_back(0);
		glGenBuffers(1, &_buffers.back());
		return _buffers.back();
	}

	unsigned Renderer::genTexture()
	{
		_textures.push_back(0);
		glGenTextures(1, &_textures.back());
		return _textures.back();
	}

	unsigned Renderer::genProgram()
	{
		_programs.push_back(glCreateProgram());
		return _programs.back();
	}

	std::vector<float> Renderer::genGrid(unsigned width, unsigned height)
	{
		auto grid = std::vector<float>(width * height * 2);

		if(width*height == 0)
			return grid;	// TODO:error handling?

		for(unsigned row = 0; row < height; ++row)
		{
			for(unsigned col = 0; col < width; ++col)
			{
				float x = static_cast<float>(col)/(width-1);
				float y = static_cast<float>(row)/(height-1);
				grid[(row*width + col)*2] = x*2.f - 1.f;
				grid[(row*width + col)*2 + 1] = y*2.f - 1.f;
			}
		}
		return grid;
	}

	std::vector<unsigned> Renderer::genGridIndices(unsigned width, unsigned height)
	{
		auto indices = std::vector<unsigned>{};
		if(width*height <= 2)
			return indices;	// TODO:error handling?

		for(unsigned y = 0; y < height-1; ++y)
		{
			for(unsigned x = 0; x < width-1; ++x)
			{
				unsigned i = y*width + x;
				indices.push_back(i+width);
				indices.push_back(i+width+1);
				indices.push_back(i);
				indices.push_back(i+width+1);
				indices.push_back(i+1);
				indices.push_back(i);
			}
		}
		return indices;
	}

	Renderer::Renderer(Renderer&& other) noexcept
		: Renderer()
	{
		swap(*this, other);
	}

	Renderer& Renderer::operator=(Renderer other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	Renderer::~Renderer()
	{
		glDeleteVertexArrays(static_cast<int>(_vaos.size()), _vaos.data());
		glDeleteBuffers(static_cast<int>(_buffers.size()), _buffers.data());
		glDeleteTextures(static_cast<int>(_textures.size()), _textures.data());
		for(auto prog : _programs)
			glDeleteProgram(prog);
	}
}
