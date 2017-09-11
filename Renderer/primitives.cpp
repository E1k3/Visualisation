#include "primitives.h"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "render_util.h"

namespace vis
{
	Primitives::Primitives(const std::vector<glm::vec3>& vertices)
	{
		using namespace render_util;

		if(vertices.empty())
		{
			Logger::error() << "Lines renderer cannot be created with an empty vertex list.";
			throw std::invalid_argument{"Lines renderer creation without vertices"};
		}

		// Data
		glBindVertexArray(_vao = gen_vertex_array());

		auto packed_vertices = std::vector<float>{};
		packed_vertices.reserve(vertices.size() * 3);
		for(const auto& v : vertices)
		{
			packed_vertices.push_back(v.x);
			packed_vertices.push_back(v.y);
			packed_vertices.push_back(v.z);
		}
		glBindBuffer(GL_ARRAY_BUFFER, _vbo = gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(packed_vertices.size() * sizeof(float)), packed_vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		_vertex_count = static_cast<int>(vertices.size());


		// Shaders
		_program = gen_program();
		auto vertex_shader = gen_shader(GL_VERTEX_SHADER);
		load_compile_shader(vertex_shader, _vertex_shaders);
		glAttachShader(_program, vertex_shader);

		auto fragment_shader = gen_shader(GL_FRAGMENT_SHADER);
		load_compile_shader(fragment_shader, _fragment_shaders);
		glAttachShader(_program, fragment_shader);

		glLinkProgram(_program);
		glUseProgram(_program);
		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, fragment_shader);

		_mvp_loc = glGetUniformLocation(_program, "mvp");
		_translations_loc = glGetUniformLocation(_program, "translations");
		_colors_loc = glGetUniformLocation(_program, "colors");
	}

	void Primitives::update(const glm::mat4& mvp)
	{
		glUseProgram(_program);

		auto color = _colors.size() < 4 ? glm::vec4{1.f} : glm::vec4{_colors[0], _colors[1], _colors[2], _colors[3]};
		while(_colors.size()/4 < _translations.size()/3)
			add_color(color);

		glUniformMatrix4fv(_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform3fv(_translations_loc, static_cast<GLsizei>(_translations.size() / 3), _translations.data());
		glUniform4fv(_colors_loc, static_cast<GLsizei>(_colors.size() / 4), _colors.data());
	}

	void Primitives::draw(GLenum mode) const
	{
		glBindVertexArray(_vao);
		glUseProgram(_program);

		glDrawArraysInstanced(mode, 0, _vertex_count, static_cast<GLsizei>(_translations.size() / 3));
	}

	void Primitives::add_translation(const glm::vec3& translation)
	{
		_translations.push_back(translation.x);
		_translations.push_back(translation.y);
		_translations.push_back(translation.z);
	}

	void Primitives::set_translations(const std::vector<glm::vec3>& translations)
	{
		_translations.clear();
		_translations.reserve(translations.size() * 3);
		for(const auto& t : translations)
			add_translation(t);
	}

	void Primitives::clear_translations()
	{
		_translations.clear();
	}

	void Primitives::set_color(const glm::vec4& color)
	{
		set_colors({color});
	}

	void Primitives::add_color(const glm::vec4& color)
	{
		_colors.push_back(color.r);
		_colors.push_back(color.g);
		_colors.push_back(color.b);
		_colors.push_back(color.a);
	}

	void Primitives::set_colors(const std::vector<glm::vec4>& colors)
	{
		_colors.clear();
		_colors.reserve(colors.size() * 3);
		for(const auto& c : colors)
			add_color(c);
	}

	void Primitives::clear_colors()
	{
		_colors.clear();
	}
}
