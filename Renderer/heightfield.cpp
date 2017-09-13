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
	{1.f, 1.f, 0.f}, {-1.f, 1.f, 0.f}}}
	{

	}

	void Heightfield::update(float /*delta_time*/, float total_time)
	{
		using namespace glm;
		constexpr vec2 mouse_speed = vec2{-0.001f, -0.001};	// Invert y-axis
		constexpr float scroll_speed = -0.1f;	// Invert scrolling
		constexpr float height_scale = .33f;	// Scale model height by this factor

		// Get input
		auto mouse_in = _input.get_cursor_offset();
		auto scroll_in = _input.get_scroll_offset_y();
		auto mouse_1_in = _input.get_button(GLFW_MOUSE_BUTTON_1);
		auto space_in = _input.get_key(GLFW_KEY_SPACE);

		// Calculate mvp
		_scale *= 1.f + scroll_in * scroll_speed;
		if(mouse_1_in)	// Only move model when dragging
		{
			auto old_position = _camera_position;
			_camera_position = rotate(_camera_position,  mouse_in.y * mouse_speed.y / _scale, cross(-_camera_position, vec3{0.f, 0.f, 1.f}));
			// Prevent flipping when looking straight up or down
			if(std::signbit(_camera_position.x) != std::signbit(old_position.x) && std::signbit(_camera_position.y) != std::signbit(old_position.y))
				_camera_position = old_position;

			_camera_position = rotateZ(_camera_position, mouse_in.x * mouse_speed.x / _scale);
		}

		// MVP calculation
		auto model = translate(scale(mat4{}, vec3{1.f, 1.f/_fields.front().aspect_ratio(), height_scale} * _scale), vec3{0.f, 0.f, -.5f});
		auto view = lookAt(_camera_position, vec3{0.f}, vec3{0.f, 0.f, 1.f});
		auto project = ortho(-1.f, 1.f, -1.f/_input.get_framebuffer_aspect_ratio(), 1.f/_input.get_framebuffer_aspect_ratio(), -20.f, 20.f);
		auto mvp = project * view * model;

		if(!mouse_1_in)	// Only move cursor when not dragging
			update_selection_cursor(mouse_in * vec2{1, -1}, view * model, _input.get_framebuffer_aspect_ratio());
		else
			update_selection_cursor(vec2{0.f}, view * model, _input.get_framebuffer_aspect_ratio());

		glUseProgram(_program);
		glUniformMatrix4fv(_mvp_loc, 1, GL_FALSE, value_ptr(mvp));
		glUniform4f(_bounds_loc, _mean_bounds.x, _mean_bounds.y, _dev_bounds.x, _dev_bounds.y);
		glUniform1f(_time_loc, total_time);

		auto field_size = vec2{_fields.front().width(), _fields.front().height()};
		auto cell_size = vec2{1.f}	/ (field_size - 1.f);
		vec4 highlight;
		if(space_in)
			highlight = (vec4{_highlight_area} / (vec4{field_size, field_size} - 1.f) + .5f * vec4{-cell_size, cell_size}) * 2.f - 1.f;
		else
			highlight = vec4{_cursor_position - .5f * cell_size, _cursor_position + .5f * cell_size} * 2.f - 1.f;
		glUniform4fv(_highlight_loc, 1, value_ptr(highlight));


		// UI
		// Update palette
		_palette.set_viewport(_input.get_framebuffer_size());
		// Update axes
		_axes.update(mvp);
		// Axes labels
		_axes_labels.set_viewport(_input.get_framebuffer_size());
		auto label_positions = std::vector<glm::vec2>();
		auto label_sizes = _axes_labels.relative_sizes();
		float corners[4][2] = {{-1.f, -1.f}, {1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}};
		for(const auto& corner : corners)
		{
			for(size_t di = 0; di < _axes_divisions.size(); ++di)
			{
				auto pos = mvp * vec4{corner[0], corner[1], (_axes_divisions[di] - _mean_bounds.x) / (_mean_bounds.y - _mean_bounds.x), 1.f};
				pos /= pos.w;
				label_positions.push_back(glm::vec2{pos});

				if(di == 0)
					label_positions.back() -= label_sizes.front();

				if(glm::clamp(pos, -1.f, 1.f) != pos)	// Clip text that should be outside of viewspace
					label_positions.back() = glm::vec2{-10.f, -10.f};
			}
			label_positions.back() -= vec2{label_sizes.back().x, 0.f};
		}
		_axes_labels.set_positions(label_positions);
		// Update cursor
		_cursor_indicator.set_translations({{_cursor_position * 2.f - 1.f, 0.f}});
		_cursor_indicator.update(mvp);
	}

	void Heightfield::draw() const
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		glBindVertexArray(_vao);
		glUseProgram(_program);

		glDrawElements(GL_TRIANGLES, _vertex_count, GL_UNSIGNED_INT, 0);
		// Cursor
		glLineWidth(2.f);
		_cursor_indicator.draw(GL_LINES);
		// Axes
		glLineWidth(1.f);
		_axes.draw(GL_LINE_LOOP);
		_axes_labels.draw();
		// Palette
		_palette.draw();
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
		_mean_bounds = glm::vec2(mean_field.minima().front(), mean_field.maxima().front());
		_dev_bounds = glm::vec2(dev_field.minima().front(), dev_field.maxima().front());

		setup_axes();
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

	void Heightfield::setup_axes()
	{
		_axes_divisions = math_util::reasonable_divisions(_mean_bounds.x, _mean_bounds.y, 10);
		_axes_divisions.insert(_axes_divisions.begin(), _mean_bounds.x);
		_axes_divisions.push_back(_mean_bounds.y);

		auto labels = std::vector<std::string>();
		for(const auto& div : _axes_divisions)
			_axes.add_translation({0.f, 0.f, (div - _mean_bounds.x) / (_mean_bounds.y - _mean_bounds.x)});

		// Add labels for each corner
		for(int i = 0; i < 4; ++i)
			for(const auto& div : _axes_divisions)
			{
				auto label_string = std::to_string(div);
				label_string.erase ( label_string.find_last_not_of('0') + 1, std::string::npos );
				label_string += '0';
				labels.push_back(label_string);
			}

		_axes_labels.set_lines(labels);

		_palette.set_bounds(_dev_bounds, 10);
	}

	glm::ivec2 Heightfield::point_under_cursor() const
	{
		return _cursor_position * glm::vec2{_fields.front().width()-1, _fields.front().height()-1} + glm::vec2{.5f};
	}
}
