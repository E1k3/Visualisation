#include "colormap.h"

#include <vector>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "render_util.h"
#include "Data/math_util.h"

namespace vis
{
	Colormap::Colormap()
	{
		using namespace render_util;

		glBindVertexArray(_vao = gen_vertex_array());

		float vertices[] = {0.f, 0.f,
							1.f, 0.f,
							0.f, 1.f,
							0.f, 1.f,
							1.f, 0.f,
							1.f, 1.f};
		glBindBuffer(GL_ARRAY_BUFFER, _vbo = gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);


		// Shaders
		_program = gen_program();

		auto vertex_shader = gen_shader(GL_VERTEX_SHADER);
		load_compile_shader(vertex_shader, _vertex_shaders);
		glAttachShader(_program, vertex_shader);

		auto fragment_shader = gen_shader(GL_FRAGMENT_SHADER);
		load_compile_shader(fragment_shader, _fragment_shaders);
		glAttachShader(_program, fragment_shader);

		glLinkProgram(_program);
		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, fragment_shader);
		glUseProgram(_program);

		_position_loc = glGetUniformLocation(_program, "origin");
		_size_loc = glGetUniformLocation(_program, "scale");

		_division_lines.set_color({.7f, .7f, .7f, 1.f});
		update();
	}

	void Colormap::set_position(const glm::vec2& position)
	{
		if(_position != position)
		{
			_position = position;
			update();
		}
	}

	void Colormap::set_size(const glm::vec2& size)
	{
		if(_size != size)
		{
			_size = size;
			update();
		}
	}

	void Colormap::set_viewport(const glm::ivec2& viewport)
	{
		if(_viewport != viewport)
		{
			_viewport = viewport;
			update();
		}
	}

	void Colormap::set_bounds(const glm::vec2& bounds, int preferred_divisions)
	{
		if(_bounds != bounds)
		{
			_bounds = bounds;

			_divisions = math_util::reasonable_divisions(_bounds.x, _bounds.y, preferred_divisions);
			_divisions.insert(_divisions.begin(), _bounds.x);
			_divisions.push_back(_bounds.y);

			auto labels = std::vector<std::string>();
			for(const auto& div : _divisions)
			{
				_division_lines.add_translation({(div - _bounds.x) / (_bounds.y - _bounds.x), 0.f, 0.f});

				auto label_string = std::to_string(div);
				label_string.erase ( label_string.find_last_not_of('0') + 1, std::string::npos );
				label_string += '0';
				labels.push_back(label_string);
			}
			labels.front() = labels.front() + "  ";
			labels.back() = ' ' + labels.back();

			_text.set_lines(labels);

			update();
		}
	}

	void Colormap::draw() const
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glUseProgram(_program);
		glBindVertexArray(_vao);

		glUniform2f(_position_loc, _position.x, _position.y);
		glUniform2f(_size_loc, _size.x, _size.y);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		_text.draw();
		glLineWidth(2.f);
		_division_lines.draw();
	}

	void Colormap::update()
	{
		_text.set_viewport(_viewport);

		auto lines_mvp = glm::translate(glm::mat4{}, glm::vec3{_position, 0.f}) * glm::scale(glm::mat4{}, glm::vec3{_size, 1.f});
		_division_lines.update(lines_mvp);

		auto text_positions = std::vector<glm::vec2>();
		for(const auto& div : _divisions)
		{
			text_positions.push_back(_position + glm::vec2{(div - _bounds.x) / (_bounds.y - _bounds.x), 1.f} * _size);
		}
		const auto& text_sizes = _text.relative_sizes();
		if(!text_sizes.empty())
		{
			text_positions.front() -= glm::vec2{text_sizes.front().x, _size.y};
			text_positions.back() -= glm::vec2{0.f, _size.y};
		}
		_text.set_positions(text_positions);
	}
}
