#include "glyph.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render_util.h"
#include "Data/math_util.h"


namespace vis
{
	Glyph::Glyph(InputManager& input, const std::vector<Field>& fields)
		: Visualization{input, fields}
	{
		_cursor_indicator.set_color({0.f, 0.f, 1.f, 1.f});
	}

	void Glyph::update(float /*delta_time*/, float /*total_time*/)
	{
		using namespace glm;
		constexpr vec2 mouse_speed = vec2(0.001f, -0.001);	// Invert y-axis
		constexpr float scroll_speed = 0.1f;	// Invert scrolling

		// Get input
		auto mouse_in = _input.get_cursor_offset();
		auto scroll_in = _input.get_scroll_offset_y();
		auto mouse_1_in = _input.get_button(GLFW_MOUSE_BUTTON_1);
		auto space_in = _input.get_key(GLFW_KEY_SPACE);

		// Calculate camera
		_scale *= 1.f + scroll_in * scroll_speed;
		if(mouse_1_in)	// Only move model when dragging
			_translation += mouse_in * mouse_speed * 1.f/_scale;

		// Preset perspectives
		if(_input.release_get_key(GLFW_KEY_1))
		{
			_scale = 1.f;
			_translation = vec2();
		}


		auto model = scale(mat4{}, vec3{1.f, 1.f/_fields.front().aspect_ratio(), 1.f});
		auto view = translate(scale(mat4{1.f}, vec3{_scale, _scale, 1.f}), vec3{_translation, 0.f});
		auto project = ortho(-1.f, 1.f, -1.f/_input.get_framebuffer_aspect_ratio(), 1.f/_input.get_framebuffer_aspect_ratio());
		auto mvp = project * view * model;

		if(!mouse_1_in)	// Only move cursor when not dragging
			update_selection_cursor(mouse_in * vec2{1, -1}, view * model, _input.get_framebuffer_aspect_ratio(), _scale);
		else
			update_selection_cursor(vec2{0.f}, view * model, _input.get_framebuffer_aspect_ratio(), _scale);

		// Set uniforms
		glUseProgram(_program);
		glUniformMatrix4fv(_mvp_loc, 1, GL_FALSE, value_ptr(mvp));
		glUniform4f(_bounds_loc, _mean_bounds.x, _mean_bounds.y, _dev_bounds.x, _dev_bounds.y);
		glUniform2i(_fieldsize_loc, _fields.front().width(), _fields.front().height());

		auto field_size = vec2{_fields.front().width(), _fields.front().height()};
		auto cell_size = vec2{1.f}	/ (field_size - 1.f);
		vec4 highlight;
		if(space_in)
			highlight = (vec4{_highlight_area} / (vec4{field_size, field_size} - 1.f) + .5f * vec4{-cell_size, cell_size}) * 2.f - 1.f;
		else
			highlight = vec4{_cursor_position - cell_size, _cursor_position} * 2.f - 1.f;
		glUniform4fv(_highlight_loc, 1, value_ptr(highlight));


		// UI
		// Update palette
		_palette.set_viewport(_input.get_framebuffer_size());
		// Update cursor
		_cursor_indicator.set_translations({glm::vec3{_cursor_position * 2.f - 1.f, 0.f} * glm::vec3{_fields.front().width(), _fields.front().height(), 1.f}});
		_cursor_indicator.update(mvp * glm::scale(glm::mat4{}, glm::vec3{1.f/_fields.front().width(), 1.f/_fields.front().height(), 1.f}));
	}

	void Glyph::draw() const
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glBindVertexArray(_vao);
		glUseProgram(_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, render_util::get_uniform_colormap_texture());

		// Draw
		glDrawArrays(GL_POINTS, 0, _vertex_count);
		// Cursor
		glDisable(GL_DEPTH_TEST);
		glLineWidth(2.f);
		_cursor_indicator.draw();
		// Palette
		_palette.draw();
	}

	void Glyph::setup_data()
	{
		using namespace render_util;

		if(_fields.size() < 2)
		{
			Logger::error() << "Glyph renderer needs at least two data fields to be created.";
			throw std::invalid_argument("Glyph renderer setup with < 2 fields");
		}
		if(!_fields[0].equal_layout(_fields[1]))
		{
			Logger::error() << "Glyph renderer needs data fields with matching format.";
			throw std::runtime_error("Glyph renderer setup with mismatched fields");
		}

		auto& mean_field = _fields[0];	// Field holding the mean for each vertex
		auto& dev_field = _fields[1];	// Field holding the deviation for each vertex
		// Vector holding the 2D position for each vertex
		auto grid = render_util::gen_grid(mean_field.width(), mean_field.height());


		glBindVertexArray(_vao = gen_vertex_array());

		// Setup VBO
		_buffers.push_back(gen_buffer());
		glBindBuffer(GL_ARRAY_BUFFER, _buffers.back());
		auto total_buffersize = grid.size() + 2*static_cast<size_t>(mean_field.area() * mean_field.point_dimension());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * total_buffersize), nullptr, GL_STATIC_DRAW);
		GLintptr buffer_offset = 0;

		// Vertex grid (position)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float)*grid.size()),	grid.data());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);
		buffer_offset += grid.size() * sizeof(float);

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

		// Set number of vertices to render
		_vertex_count = mean_field.area();

		// Set data bounds
		_mean_bounds = glm::vec2(math_util::combined_minima(mean_field, dev_field).front(), math_util::combined_maxima(mean_field, dev_field).front());
		_dev_bounds = glm::vec2(0, _mean_bounds.y - _mean_bounds.x);

		_palette.set_bounds(_mean_bounds, 10);
	}

	void Glyph::setup_shaders()
	{
		using namespace render_util;

		// Create program
		_program = gen_program();

		// Create, load and compile shaders
		auto vertex_shader = gen_shader(GL_VERTEX_SHADER);
		render_util::load_compile_shader(vertex_shader, _vertex_shaders);
		glAttachShader(_program, vertex_shader);

		auto geometry_shader = gen_shader(GL_GEOMETRY_SHADER);
		render_util::load_compile_shader(geometry_shader, _geometry_shaders);
		glAttachShader(_program, geometry_shader);

		auto fragment_shader = gen_shader(GL_FRAGMENT_SHADER);
		render_util::load_compile_shader(fragment_shader, _fragment_shaders);
		glAttachShader(_program, fragment_shader);

		// Link and use program and free up memory
		glLinkProgram(_program);
		glUseProgram(_program);
		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, geometry_shader);
		glDetachShader(_program, fragment_shader);

		// Configure uniforms
		_mvp_loc = glGetUniformLocation(_program, "mvp");
		_bounds_loc = glGetUniformLocation(_program, "bounds");
		_highlight_loc = glGetUniformLocation(_program, "highlight_area");
		_fieldsize_loc = glGetUniformLocation(_program, "field_size");
	}

	glm::ivec2 Glyph::point_under_cursor() const
	{
		return _cursor_position * glm::vec2{_fields.front().width()-1, _fields.front().height()-1};
	}
}
