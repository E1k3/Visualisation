#include "lines.h"

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "render_util.h"

namespace vis
{
	Lines::Lines(const std::vector<glm::vec3>& vertices)
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
	}

	void Lines::update(const glm::mat4& mvp)
	{
		glUseProgram(_program);
		glUniformMatrix4fv(_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform3fv(_translations_loc, static_cast<GLsizei>(_translations.size() / 3), _translations.data());
	}

	void Lines::draw() const
	{
		glBindVertexArray(_vao);
		glUseProgram(_program);

		glDrawArraysInstanced(GL_LINES, 0, _vertex_count, static_cast<GLsizei>(_translations.size() / 3));
	}

	void Lines::add_translation(const glm::vec3& translation)
	{
		_translations.push_back(translation.x);
		_translations.push_back(translation.y);
		_translations.push_back(translation.z);
	}

	void Lines::set_translations(const std::vector<glm::vec3>& translations)
	{
		_translations.clear();
		_translations.reserve(translations.size() * 3);
		for(const auto& t : translations)
		{
			_translations.push_back(t.x);
			_translations.push_back(t.y);
			_translations.push_back(t.z);
		}
	}
}
