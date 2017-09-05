#include "heightfield.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "render_util.h"
#include "Data/math_util.h"

namespace vis
{
	Heightfield::Heightfield(InputManager& input, const std::vector<Field>& fields)
		: Visualisation{input, fields},
		  _axes{{
	{-1.f, -1.f, 0.f}, {1.f, -1.f, 0.f},
	{-1.f, -1.f, 0.f}, {-1.f, 1.f, 0.f},
	{-1.f, 1.f, 0.f}, {1.f, 1.f, 0.f},
	{1.f, -1.f, 0.f}, {1.f, 1.f, 0.f}}}
	{
		_axes.set_translations({{0.f, 0.f, 0.f},
								{0.f, 0.f, .1f},
								{0.f, 0.f, .2f},
								{0.f, 0.f, .3f},
								{0.f, 0.f, .4f},
								{0.f, 0.f, .5f},
								{0.f, 0.f, .6f},
								{0.f, 0.f, .7f},
								{0.f, 0.f, .8f},
								{0.f, 0.f, .9f},
								{0.f, 0.f, 1.f}});
	}

	void Heightfield::update(float /*delta_time*/, float total_time)
	{
		using namespace glm;
		constexpr vec2 mouse_speed = vec2{-0.001f, -0.001};	// Invert y-axis
		constexpr float scroll_speed = -0.1f;	// Invert scrolling
		constexpr float height_scale = .25f;	// Scale model height by this factor

		// Get input
		auto mouse_in = _input.get_cursor_offset();
		auto scroll_in = _input.get_scroll_offset_y();
		auto mouse_1_in = _input.get_button(GLFW_MOUSE_BUTTON_1);

		// Calculate mvp
		_scale *= 1.f + scroll_in * scroll_speed;
		if(mouse_1_in)	// Only move model when dragging
		{
			auto old_position = _camera_position;
			_camera_position = rotate(_camera_position,  mouse_in.y * mouse_speed.y, cross(-_camera_position, vec3{0.f, 0.f, 1.f}));
			// Prevent flipping when looking straight up or down
			if(std::signbit(_camera_position.x) != std::signbit(old_position.x) && std::signbit(_camera_position.y) != std::signbit(old_position.y))
				_camera_position = old_position;

			_camera_position = rotateZ(_camera_position, mouse_in.x * mouse_speed.x);
		}

		// MVP calculation
		auto model = translate(scale(mat4{}, vec3{1.f, 1.f/_fields.front().aspect_ratio(), height_scale} * _scale), vec3{0.f, 0.f, -.5f});
		auto view = lookAt(_camera_position, vec3{0.f}, vec3{0.f, 0.f, 1.f});
		auto project = ortho(-1.f, 1.f, -1.f/_input.get_framebuffer_aspect_ratio(), 1.f/_input.get_framebuffer_aspect_ratio(), -20.f, 20.f);
		auto mvp = project * view * model;

		if(!mouse_1_in)	// Only move cursor when not dragging
			update_selection_cursor(mouse_in * vec2{1, -1}, view * model, project, _input.get_framebuffer_aspect_ratio());
		else
			update_selection_cursor(vec2{0.f}, view * model, project, _input.get_framebuffer_aspect_ratio());

		glUseProgram(_program);
		glUniformMatrix4fv(_mvp_loc, 1, GL_FALSE, value_ptr(mvp));
		glUniform4f(_bounds_loc, _mean_bounds.x, _mean_bounds.y, _dev_bounds.x, _dev_bounds.y);
		glUniform1f(_time_loc, total_time);

		auto cell_width = vec2{1.f}	/ vec2{_fields.front().width(), _fields.front().height()};
		auto highlight = vec4{_selection_cursor - .5f * cell_width, _selection_cursor + .5f * cell_width} * 2.f - 1.f;
		glUniform4fv(_highlight_loc, 1, value_ptr(highlight));

		_axes.update(mvp);
	}

	void Heightfield::draw() const
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glBindVertexArray(_vao);
		glUseProgram(_program);

		glDrawElements(GL_TRIANGLES, _vertex_count, GL_UNSIGNED_INT, 0);
		_axes.draw();
		_cursor_line.draw();
	}

	void Heightfield::setup_data()
	{
		using namespace render_util;
		if(_fields.size() < 2)
		{
			Logger::error() << "Heightfield renderer needs at least two data fields to be created.";
			throw std::invalid_argument("Heightfield renderer setup with < 2 fields");
		}
		if(!_fields[0].equal_layout(_fields[1]))
		{
			Logger::error() << "Heightfield renderer needs data fields with matching format.";
			throw std::runtime_error("Heightfield renderer setup with mismatched fields");
		}

		auto& mean_field = _fields[0];	// Field holding the mean for each vertex
		auto& dev_field = _fields[1];	// Field holding the deviation for each vertex
		// Vector holding the 2D position for each vertex
		auto grid = gen_grid(mean_field.width(), mean_field.height());

		glBindVertexArray(_vao = gen_vertex_array());

		// Setup VBO
		_buffers.push_back(gen_buffer());
		glBindBuffer(GL_ARRAY_BUFFER, _buffers.back());
		auto total_buffersize = grid.size() + 2*static_cast<size_t>(mean_field.area() * mean_field.point_dimension());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(float) * total_buffersize), nullptr, GL_STATIC_DRAW);
		GLintptr buffer_offset = 0;

		// Vertex grid (position)
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float)*grid.size()),	grid.data());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
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

		// Infices (element buffer)
		auto indices = render_util::gen_grid_indices(mean_field.width(), mean_field.height());
		_buffers.push_back(gen_buffer());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers.back());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(unsigned)*indices.size()), indices.data(), GL_STATIC_DRAW);

		// Set number of vertex indices to render
		_vertex_count = static_cast<int>(indices.size());

		// Set data bounds
		std::tie(_mean_bounds.x, _mean_bounds.y) = math_util::round_interval(mean_field.minima()[0], mean_field.maxima()[0]);
		std::tie(_dev_bounds.x, _dev_bounds.y) = math_util::round_interval(dev_field.minima()[0], dev_field.maxima()[0]);
	}

	void Heightfield::setup_shaders()
	{
		using namespace render_util;
		// Create program
		_program = gen_program();

		// Create, load and compile shaders
		auto vertex_shader = gen_shader(GL_VERTEX_SHADER);
		load_compile_shader(vertex_shader, _vertex_shaders);
		glAttachShader(_program, vertex_shader);

		auto fragment_shader = gen_shader(GL_FRAGMENT_SHADER);
		load_compile_shader(fragment_shader, _fragment_shaders);
		glAttachShader(_program, fragment_shader);

		// Link and use program and free up memory
		glLinkProgram(_program);
		glUseProgram(_program);
		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, fragment_shader);

		// Configure uniforms
		_mvp_loc = glGetUniformLocation(_program, "mvp");
		_bounds_loc = glGetUniformLocation(_program, "bounds");
		_highlight_loc = glGetUniformLocation(_program, "highlight_area");
		_time_loc = glGetUniformLocation(_program, "time");
	}
}
