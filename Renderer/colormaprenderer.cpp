#include "colormaprenderer.h"

#include <vector>
#include <algorithm>

#include "Data/math_util.h"

namespace vis
{
	ColormapRenderer::ColormapRenderer()
		: Renderer{}
	{
		_vao = gen_vao();
		glBindVertexArray(_vao);

		float vertices[] = {0.f, 0.f,
							1.f, 0.f,
							0.f, 1.f,
							0.f, 1.f,
							1.f, 0.f,
							1.f, 1.f};
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/colormap_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/colormap_fs.glsl",
											"/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);
		_program = gen_program();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);
		_position_uniform = glGetUniformLocation(_program, "origin");
		_size_uniform = glGetUniformLocation(_program, "scale");
		_viewport_uniform = glGetUniformLocation(_program, "viewport");
		_division_uniform = glGetUniformLocation(_program, "divisions");

		update();
	}

	void ColormapRenderer::set_position(const glm::vec2& position)
	{
		if(_position != position)
		{
			_position = position;
			update();
		}
	}

	void ColormapRenderer::set_size(const glm::vec2& size)
	{
		if(_size != size)
		{
			_size = size;
			update();
		}
	}

	void ColormapRenderer::set_viewport(const glm::ivec2& viewport)
	{
		if(_viewport != viewport)
		{
			_viewport = viewport;
			update();
		}
	}

	void ColormapRenderer::set_bounds(const glm::vec2& bounds)
	{
		if(_bounds != bounds)
		{
			_bounds = bounds;
			update();
		}
	}

	void ColormapRenderer::set_divisions(int divisions)
	{
		if(_divisions != divisions)
		{
			_divisions = std::max(divisions, 1);
			update();
		}
	}

	void ColormapRenderer::draw(float delta_time, float total_time)
	{
		auto depthtest = glIsEnabled(GL_DEPTH_TEST);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(_program);
		glBindVertexArray(_vao);

		glUniform2f(_position_uniform, _position.x, _position.y);
		glUniform2f(_size_uniform, _size.x, _size.y);
		glUniform2i(_viewport_uniform, _viewport.x, _viewport.y);
		glUniform1i(_division_uniform, _divisions);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		if(depthtest)
			glEnable(GL_DEPTH_TEST);

		_text.draw(delta_time, total_time);
	}

	void ColormapRenderer::update()
	{
		_text_lines = std::vector<std::string>(static_cast<size_t>(_divisions+1));
		_text_positions = std::vector<glm::vec2>(static_cast<size_t>(_divisions+1));

		for(int i = 0; i <= _divisions; ++i)
		{
			_text_lines[static_cast<size_t>(i)] = std::to_string(_bounds.x + ((_bounds.y - _bounds.x) / _divisions) * i);
			_text_positions[static_cast<size_t>(i)] = glm::vec2(_position.x + (_size.x / _divisions) * i, _position.y + _size.y * 1.1f);
		}
		_text.set_lines(_text_lines);
		_text.set_positions(_text_positions);
		_text.set_viewport(_viewport);
	}
}
