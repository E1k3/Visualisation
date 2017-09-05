#include "render_util.h"

#include <fstream>
#include <algorithm>

#include "logger.h"

namespace vis
{
	void render_util::load_compile_shader(GLuint id, const std::vector<std::string>& paths)
	{
		// Load code
		auto sources = std::vector<std::string>{};
		for(const auto& path : paths)
		{
			Logger::debug() << "Loading shader from source: " << path;
			sources.push_back("");
			auto ifs = std::ifstream(path);
			std::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(),
					  std::back_inserter(sources.back()));
			sources.back().push_back('\0');
		}
		auto source_ptrs = std::vector<const char*>{};
		std::transform(sources.begin(), sources.end(), std::back_inserter(source_ptrs),
					   [] (const auto& s) { return s.c_str(); });

		glShaderSource(id, static_cast<GLsizei>(source_ptrs.size()), source_ptrs.data(), nullptr);
		glCompileShader(id);

		// Check for errors
		GLint success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if(success != GL_TRUE)
		{
			int logsize = 0;
			std::vector<char> log;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logsize);
			log.resize(static_cast<unsigned>(logsize));
			glGetShaderInfoLog(id, logsize, nullptr, &log[0]);

			Logger::error() << "Shader did not compile." << &log[0];
			throw std::runtime_error("Load Shader Error");
		}
	}

	std::vector<float> render_util::gen_grid(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::error() << "Grid generation using negative dimensions.\n"
							<< "width: " << width << " height: " << height;
			throw std::length_error("Negative grid generation dimensions");
		}


		auto grid = std::vector<float>(static_cast<size_t>(width * height * 2));

		if(grid.empty())
			return grid;	// TODO:error handling?

		for(int row = 0; row < height; ++row)
		{
			for(int col = 0; col < width; ++col)
			{
				float x = col/(width-1.f);
				float y = row/(height-1.f);
				grid[static_cast<size_t>(row*width + col)*2]    	= x*2.f - 1.f;
				grid[static_cast<size_t>(row*width + col)*2 + 1]	= y*2.f - 1.f;
			}
		}
		return grid;
	}

	std::vector<unsigned> render_util::gen_grid_indices(int width, int height)
	{
		if(width < 0 || height < 0)
		{
			Logger::error() << "Grid index generation using negative dimensions.\n"
							<< "width: " << width << " height: " << height;
			throw std::length_error("Negative grid index generation dimensions");
		}

		auto indices = std::vector<unsigned>{};
		if(width*height <= 2)
			return indices;	// TODO:error handling?

		for(int y = 0; y < height-1; ++y)
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
		return indices;
	}

	VertexArray render_util::gen_vertex_array()
	{
		GLuint id = 0;
		glGenVertexArrays(1, &id);
		return VertexArray(id);
	}

	Buffer render_util::gen_buffer()
	{
		GLuint id = 0;
		glGenBuffers(1, &id);
		return Buffer(id);
	}

	Texture render_util::gen_texture()
	{
		GLuint id = 0;
		glGenTextures(1, &id);
		return Texture(id);
	}

	Program render_util::gen_program()
	{
		return Program{glCreateProgram()};
	}

	Shader render_util::gen_shader(GLenum shadertype)
	{
		return Shader(glCreateShader(shadertype));
	}
}
