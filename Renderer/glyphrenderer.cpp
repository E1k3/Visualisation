#include "glyphrenderer.h"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "Data/math_util.h"
#include "application.h"
#include "render_util.h"

namespace vis
{
	GlyphRenderer::GlyphRenderer(const std::vector<Field>& fields, InputManager& input)
		: Renderer{},
		  _input{input},
		  _fields{fields}
	{
		if(fields.size() <= 1)
		{
			Logger::error() << "Glyph renderer creation from less than 2 fields is impossible.";
			throw std::invalid_argument("Glyph renderer creation with < 2 fields");
		}
		if(fields.size() == 2)
			init_gaussian();
		if(fields.size() >= 3)
			init_gmm();
	}


	void GlyphRenderer::init_gaussian()
	{
		auto mean_field = _fields[0];
		auto dev_field = _fields[1];

		if(!mean_field.equal_layout(dev_field))
		{
			Logger::error() << "The mean and deviation fields have differing sizes.";
			throw std::runtime_error("Glyph rendering error");
			//TODO:ERROR handling. mean and dev field have differing size.
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);
		auto grid = render_util::gen_grid(mean_field.width(), mean_field.height());
		// Setup VBO
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		auto total_buffersize = grid.size() + static_cast<size_t>(mean_field.area() * mean_field.point_dimension()
																  + dev_field.area() * dev_field.point_dimension());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * total_buffersize), nullptr, GL_STATIC_DRAW);
		GLintptr buffer_offset = 0;

		// Vertex grid (position)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float)*grid.size()),	grid.data());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		buffer_offset += sizeof(float)*grid.size();

		// Mean (ring)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float))*mean_field.area()*mean_field.point_dimension(), mean_field.data().data());
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), reinterpret_cast<void*>(buffer_offset));
		glEnableVertexAttribArray(1);
		buffer_offset += static_cast<GLsizeiptr>(sizeof(float))*mean_field.area()*mean_field.point_dimension();

		// Deviation (dot & background)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float))*dev_field.area()*dev_field.point_dimension(), dev_field.data().data());
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, dev_field.point_dimension()*static_cast<int>(sizeof(float)), reinterpret_cast<void*>(buffer_offset));
		glEnableVertexAttribArray(2);
		buffer_offset += static_cast<GLsizeiptr>(sizeof(float))*dev_field.area()*dev_field.point_dimension();

		_num_vertices = mean_field.area();

		Logger::debug() << grid.size() << ".." << mean_field.area() << "-.-" << dev_field.area();


		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/glyph_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto geometry_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/glyph_gs.glsl"},	//TODO:change location to relative
										   GL_GEOMETRY_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/glyph_fs.glsl",
											"/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);

		_program = gen_program();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, geometry_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, geometry_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(geometry_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);

		// Get uniform locations and set constant uniforms
		glUniform2i(glGetUniformLocation(_program, "field_size"), mean_field.width(), mean_field.height());
		_mvp_uniform = glGetUniformLocation(_program, "mvp");
		_bounds_uniform = glGetUniformLocation(_program, "bounds");
		_highlight_uniform = glGetUniformLocation(_program, "highlight_area");

		// Set interval bounds
		std::tie(_bounds.x, _bounds.y) = math_util::round_interval(mean_field.minima()[0], mean_field.maxima()[0]);
		std::tie(_bounds.z, _bounds.w) = math_util::round_interval(dev_field.minima()[0], dev_field.maxima()[0]);
	}

	void GlyphRenderer::init_gmm()
	{
		auto mean_field = _fields[0];
		auto dev_field = _fields[1];
		auto weight_field = _fields[2];

		if(!mean_field.equal_layout(dev_field) || !mean_field.equal_layout(weight_field))
		{
			Logger::error() << "The mean, deviation and weight fields have differing sizes.";
			throw std::runtime_error("Glyph rendering error.");
			//TODO:ERROR handling. mean and dev field have differing size.
		}
		if(mean_field.point_dimension() != 4)
		{
			Logger::error() << "Fields for GMM rendering do not have a point dimension of 4.";
			throw std::runtime_error("Glyph rendering error.");
			//TODO:ERROR handling. mean and dev field have differing size.
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);

		// Grid (position)
		auto grid = gen_grid(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		_num_vertices = mean_field.area();

		// Mean (ring)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float))*mean_field.area()*mean_field.point_dimension(),
					 mean_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(1);

		// Standard Deviation (circle and background)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float))*dev_field.area()*dev_field.point_dimension(),
					 dev_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, dev_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(2);

		// Weight (proportions)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float))*weight_field.area()*weight_field.point_dimension(),
					 weight_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, weight_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(3);

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto geometry_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_gs.glsl"},	//TODO:change location to relative
										   GL_GEOMETRY_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_fs_2.glsl",
											"/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);

		_program = gen_program();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, geometry_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, geometry_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(geometry_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);

		// Get uniform locations and set constant uniforms
		glUniform2i(glGetUniformLocation(_program, "field_size"), mean_field.width(), mean_field.height());
		_mvp_uniform = glGetUniformLocation(_program, "mvp");
		_bounds_uniform = glGetUniformLocation(_program, "bounds");
		_highlight_uniform = glGetUniformLocation(_program, "highlight_area");

		// Set interval bounds
		std::tie(_bounds.x, _bounds.y) = math_util::round_interval(mean_field.minimum(), mean_field.maximum());
		std::tie(_bounds.z, _bounds.w) = math_util::round_interval(dev_field.minimum(), dev_field.maximum());
	}

	void GlyphRenderer::draw(float delta_time, float total_time)
	{
		glBindVertexArray(_vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glUseProgram(_program);

		// Input handling
		using namespace glm;
		const float mousespeed = 0.001f;
		const float scrollspeed = -0.1f;
		if(!_input.get_button(GLFW_MOUSE_BUTTON_1))
			_input.reset_cursor_offsets();
		auto trans_offset = _input.get_cursor_offset();
		trans_offset.x = -trans_offset.x;
		_translate += vec3(trans_offset * mousespeed * 1.f/_scale, 0.f);
		auto scale_offset = _input.get_scroll_offset_y();
		_scale *= 1.f + scale_offset*scrollspeed;

		auto framebuffer_size = _input.get_framebuffer_size();

		// MVP calculation
		auto model = scale(mat4{1.f}, vec3{192.f/96.f * framebuffer_size.y/framebuffer_size.x, 1.f, 1.f});
		auto view = glm::scale(glm::mat4{1.f}, vec3{_scale, _scale, 1.f}) * glm::translate(glm::mat4{1.f}, _translate);
		auto proj = mat4{1.f};
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));
		glUniform4f(_bounds_uniform, _bounds.x, _bounds.y, _bounds.z, _bounds.w);

		if(_input.get_key(GLFW_KEY_SPACE))
		{
			auto& h = Application::study_highlights[Application::study_select];
			glUniform4f(_highlight_uniform,
						(h[0]-.5f) * 2.f / _fields.front().width() - 1.f,
					(h[1]-.5f) * 2.f / _fields.front().height() - 1.f,
					(h[2]+.5f) * 2.f / _fields.front().width() - 1.f,
					(h[3]+.5f) * 2.f / _fields.front().height() - 1.f);
		}
		else
		{//TODO
			glUniform4f(_highlight_uniform, 1.f, 1.f, 1.f, 1.f);
		}

		// Draw
		glDrawArrays(GL_POINTS, 0, _num_vertices);

		// Render palette
		_palette.set_bounds(_bounds);
		_palette.set_viewport(framebuffer_size);
		//_palette.draw(delta_time, total_time);
	}
}
