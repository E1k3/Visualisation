#include "glyph.h"

#include "GLFW/glfw3.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render_util.h"
#include "Data/math_util.h"


namespace vis
{
	Glyph::Glyph(InputManager& input, const std::vector<Field>& fields)
		: Visualisation{input, fields}
	{
		setup_data();
		setup_shaders();
	}

	void Glyph::update(float /*delta_time*/, float /*total_time*/)
	{
		using namespace glm;
		constexpr vec2 mouse_speed = vec2(0.001f, -0.001);	// Invert y-axis
		constexpr float scroll_speed = -0.1f;	// Invert scrolling

		// Get input
		auto mouse_in = _input.get_cursor_offset();
		auto scroll_in = _input.get_scroll_offset_y();
		auto mouse_1_in = _input.get_button(GLFW_MOUSE_BUTTON_1);

		_scale *= 1.f + scroll_in * scroll_speed;
		if(mouse_1_in)	// Only move model when dragging
			_translation += mouse_in * mouse_speed * 1.f/_scale;

		auto model = scale(mat4{}, vec3{1.f, 1.f/_fields.front().aspect_ratio(), 1.f});
		auto view = translate(scale(mat4{1.f}, vec3{_scale, _scale, 1.f}), vec3{_translation, 0.f});
		auto project = ortho(-1.f, 1.f, -1.f/_input.get_framebuffer_aspect_ratio(), 1.f/_input.get_framebuffer_aspect_ratio());
		auto mvp = project * view * model;

		if(!mouse_1_in)	// Only move cursor when not dragging
			update_selection_cursor(mouse_in * vec2(1, -1), view * model);

		glUseProgram(_program);
		glUniformMatrix4fv(_mvp_loc, 1, GL_FALSE, value_ptr(mvp));
		glUniform4f(_bounds_loc, _mean_bounds.x, _mean_bounds.y, _dev_bounds.x, _dev_bounds.y);

		auto cell_width = vec2(1.f)	/ vec2(_fields.front().width(), _fields.front().height());
		auto highlight = vec4(_selection_cursor - .5f * cell_width, _selection_cursor + .5f * cell_width) * 2.f - 1.f;
		glUniform4fv(_highlight_loc, 1, value_ptr(highlight));
	}

	void Glyph::draw() const
	{
		glBindVertexArray(_vao);
		glUseProgram(_program);

		// Draw
		glDrawArrays(GL_POINTS, 0, _vertex_count);
	}

	void Glyph::setup_data()
	{
		if(_fields.size() < 2)
		{
			Logger::error() << "Glyph renderer needs at least two data fields to be created.";
			throw std::invalid_argument("Glyph renderer creation with < 2 fields");
		}
		if(!_fields[0].equal_layout(_fields[1]))
		{
			Logger::error() << "Glyph renderer needs data fields with matching format.";
			throw std::runtime_error("Glyph rendering error");
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
		glBufferSubData(GL_ARRAY_BUFFER, buffer_offset, static_cast<GLsizeiptr>(sizeof(float)*grid.size()),	&grid[0]);
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

		// Set number of vertices to render
		_vertex_count = mean_field.area();

		// Set data bounds
		std::tie(_mean_bounds.x, _mean_bounds.y) = math_util::round_interval(mean_field.minima()[0], mean_field.maxima()[0]);
		std::tie(_dev_bounds.x, _dev_bounds.y) = math_util::round_interval(dev_field.minima()[0], dev_field.maxima()[0]);
	}

	void Glyph::setup_shaders()
	{
		// Create program
		_program = gen_program();

		// Create, load and compile shaders
		auto vertex_shader = gen_shader(GL_VERTEX_SHADER);
		render_util::load_compile_shader(vertex_shader, {"/home/eike/Documents/Code/Visualisation/Shader/glyph_vs.glsl"});
		glAttachShader(_program, vertex_shader);

		auto geometry_shader = gen_shader(GL_GEOMETRY_SHADER);
		render_util::load_compile_shader(geometry_shader, {"/home/eike/Documents/Code/Visualisation/Shader/glyph_gs.glsl"});
		glAttachShader(_program, geometry_shader);

		auto fragment_shader = gen_shader(GL_FRAGMENT_SHADER);
		render_util::load_compile_shader(fragment_shader, {"/home/eike/Documents/Code/Visualisation/Shader/glyph_fs.glsl",
														   "/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"});
		glAttachShader(_program, fragment_shader);

		// Link and use program and free up memory
		glLinkProgram(_program);
		glUseProgram(_program);
		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, geometry_shader);
		glDetachShader(_program, fragment_shader);

		// Configure uniforms
		glUniform2i(glGetUniformLocation(_program, "field_size"), _fields[0].width(), _fields[0].height());
		_mvp_loc = glGetUniformLocation(_program, "mvp");
		_bounds_loc = glGetUniformLocation(_program, "bounds");
		_highlight_loc = glGetUniformLocation(_program, "highlight_area");
	}
}
