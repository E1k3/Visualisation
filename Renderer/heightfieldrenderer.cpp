#include "heightfieldrenderer.h"

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "logger.h"

namespace vis
{
	HeightfieldRenderer::HeightfieldRenderer(const std::vector<Field>& fields, InputManager& input)
		: Renderer{},
		  _fields{fields},
		  _input{input}
	{
		if(fields.size() < 2)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Heightfield renderer creation with < 2 fields not possible.";
			throw std::invalid_argument("Heightfield renderer created using < 2 fields");
		}
		else if(fields.size() == 2)
			init_gaussian();
		else
			init_gmm();
		init_scale_planes(20);
	}

	void HeightfieldRenderer::init_gaussian()
	{
		auto mean_field = _fields[0];
		auto var_field = _fields[1];

		if(!mean_field.equal_layout(var_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean and variance fields have differing sizes.";
			throw std::runtime_error("Heightfield rendering error.");
			//TODO:ERROR handling. mean and var field have differing size.
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);

		// Grid (position)
		auto grid = gen_grid(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
		glEnableVertexAttribArray(0);

		// Mean (color)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*mean_field.area()*mean_field.point_dimension(),
					 mean_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(1);

		// Variance (height)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*var_field.area()*var_field.point_dimension(),
					 var_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, var_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(2);

		// Indices (element buffer)
		auto indices = gen_grid_indices(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);
		_num_vertices = static_cast<int>(indices.size());

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",
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
		_mvp_uniform = glGetUniformLocation(_program, "mvp");
		_bounds_uniform = glGetUniformLocation(_program, "bounds");
		_bounds = glm::vec4(mean_field.minima()[0], mean_field.maxima()[0], var_field.minima()[0], var_field.maxima()[0]);
	}

	void HeightfieldRenderer::init_gmm()
	{
		auto mean_field = _fields[0];
		auto var_field = _fields[1];
		auto weight_field = _fields[2];

		if(!mean_field.equal_layout(var_field) || !var_field.equal_layout(weight_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean, variance and weight fields have differing layouts.";
			throw std::invalid_argument("Heightfield creation using different fields");
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);

		// Grid (position)
		auto grid = gen_grid(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
		glEnableVertexAttribArray(0);

		// Mean (color)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*mean_field.area()*mean_field.point_dimension(),
					 mean_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(1);

		// Variance (height)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*var_field.area()*var_field.point_dimension(),
					 var_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, var_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(2);

		// Weight
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*weight_field.area()*weight_field.point_dimension(),
					 weight_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, weight_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(3);

		// Indices (element buffer)
		auto indices = gen_grid_indices(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);
		_num_vertices = static_cast<int>(indices.size());

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_fs.glsl",
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
		_mvp_uniform = glGetUniformLocation(_program, "mvp");
		_bounds_uniform = glGetUniformLocation(_program, "bounds");
		_time_uniform = glGetUniformLocation(_program, "time");
		_bounds = glm::vec4(mean_field.minimum(), mean_field.maximum(), var_field.minimum(), var_field.maximum());
	}

	void HeightfieldRenderer::init_scale_planes(int divisions)
	{
		_scale_plane_vao = gen_vao();
		glBindVertexArray(_scale_plane_vao);

		divisions = std::max(divisions, 0);
		_scale_plane_num_vertices = (divisions + 1) * 4 * 2;
		auto vertices = std::vector<float>(static_cast<size_t>(_scale_plane_num_vertices));
		for(int i = 0; i <= divisions; ++i)
		{
			vertices[static_cast<size_t>(i * 8 + 0)] = i * 2.f / divisions - 1.f;
			vertices[static_cast<size_t>(i * 8 + 1)] = -1.f;
			vertices[static_cast<size_t>(i * 8 + 2)] = i * 2.f / divisions - 1.f;
			vertices[static_cast<size_t>(i * 8 + 3)] = 1.f;

			vertices[static_cast<size_t>(i * 8 + 4)] = -1.f;
			vertices[static_cast<size_t>(i * 8 + 5)] = i * 2.f / divisions - 1.f;
			vertices[static_cast<size_t>(i * 8 + 6)] = 1.f;
			vertices[static_cast<size_t>(i * 8 + 7)] = i * 2.f / divisions - 1.f;
		}

		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(sizeof(float) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/scale_plane_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/scale_plane_fs.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);
		_scale_plane_program = gen_program();
		glAttachShader(_scale_plane_program, vertex_shader);
		glAttachShader(_scale_plane_program, fragment_shader);
		glLinkProgram(_scale_plane_program);

		glDetachShader(_scale_plane_program, vertex_shader);
		glDetachShader(_scale_plane_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_scale_plane_program);
		_scale_plane_mvp_uniform = glGetUniformLocation(_scale_plane_program, "mvp");
		_scale_plane_count_uniform = glGetUniformLocation(_scale_plane_program, "count");
		_scale_plane_opacity_uniform = glGetUniformLocation(_scale_plane_program, "opacity");
	}

	void HeightfieldRenderer::draw_scale_planes(const glm::mat4& mvp, int count)
	{
		// Save old OpenGL state and setup
		auto blending = glIsEnabled(GL_BLEND);
		int blendfunc;
		glGetIntegerv(GL_SRC_ALPHA, &blendfunc);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Setup and render planes
		glUseProgram(_scale_plane_program);
		glBindVertexArray(_scale_plane_vao);
		count = std::max(count, 1);
		glUniformMatrix4fv(_scale_plane_mvp_uniform, 1, GL_FALSE, glm::value_ptr(mvp));
		glUniform1i(_scale_plane_count_uniform, count);
		glUniform1f(_scale_plane_opacity_uniform, 0.1f);
		glDrawArraysInstanced(GL_LINES, 0, _scale_plane_num_vertices, count);

		// Reset OpenGL state
		if(!blending)
			glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, static_cast<GLenum>(blendfunc));

		// Setup and render labels
		auto lines = std::vector<std::string>(static_cast<size_t>(count));
		auto positions = std::vector<glm::vec2>(static_cast<size_t>(count));
		for(int i = 0; i < count; ++i)
		{
			lines[static_cast<size_t>(i)] = std::to_string(_bounds.x + i * (_bounds.y - _bounds.x) / (count - 1));
			auto pos = mvp * glm::vec4{0.f, 0.f, static_cast<float>(i) / (count - 1), 1.f};
			positions[static_cast<size_t>(i)] = glm::vec2{pos / pos.w};
		}

		_scale_plane_text.set_lines(lines);
		_scale_plane_text.set_positions(positions);
		_scale_plane_text.draw(0.f, 0.f);
	}

	void HeightfieldRenderer::draw(float delta_time, float total_time)
	{
		glBindVertexArray(_vao);
		glUseProgram(_program);

		// Input handling
		using namespace glm;
		constexpr float mousespeed = 0.001f;
		constexpr float scrollspeed = 0.05f;
		auto cursor_x = _input.get_cursor_offset_x();
		auto cursor_y = _input.get_cursor_offset_y();
		auto old_cam_pos = _cam_position;
		_cam_position = rotate(_cam_position,  cursor_y*mousespeed, cross(-_cam_position, vec3(0.f, 0.f, 1.f)));
		// Prevent flipping when reaching cam (x,y)==(0,0)
		if((_cam_position.x > 0.f && old_cam_pos.x < 0.f && _cam_position.y >= 0.f && old_cam_pos.y < 0.f)
				|| (_cam_position.x > 0.f && old_cam_pos.x < 0.f && _cam_position.y < 0.f && old_cam_pos.y > 0.f)
				|| (_cam_position.x < 0.f && old_cam_pos.x > 0.f && _cam_position.y > 0.f && old_cam_pos.y < 0.f)
				|| (_cam_position.x < 0.f && old_cam_pos.x > 0.f && _cam_position.y < 0.f && old_cam_pos.y > 0.f))
			_cam_position = old_cam_pos;
		_cam_position = rotateZ(_cam_position, cursor_x*mousespeed);
		_cam_position = _cam_position*(1 - _input.get_scroll_offset_y() * scrollspeed);


		auto framebuffer_size = _input.get_framebuffer_size();
		if(framebuffer_size == glm::ivec2{0})	// Prevent aspect ratio = 0/0
			framebuffer_size = glm::ivec2{1};

		// MVP calculation
		constexpr float height_scale = .5f;
		auto model = scale(mat4{1.f}, vec3{_fields.front().aspect_ratio(), 1.f, height_scale});
		auto view = lookAt(_cam_position, vec3(0.f), vec3{0.f, 0.f, 1.f});
		auto proj = perspective(radians(45.f), static_cast<float>(framebuffer_size.x)/framebuffer_size.y, .2f, 20.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		glUniform4f(_bounds_uniform, _bounds.x, _bounds.y, _bounds.z, _bounds.w);

		// Set time uniform
		if(_time_uniform != -1)
			glUniform1f(_time_uniform, total_time/2.f);

		// Draw
		glDrawElements(GL_TRIANGLES, _num_vertices, GL_UNSIGNED_INT, 0);

		_scale_plane_text.set_viewport(framebuffer_size);
		draw_scale_planes(mvp, height_scale*10);

		// Render palette
		_palette.set_divisions(8);
		_palette.set_bounds({_bounds.z, _bounds.w});
		_palette.set_viewport(framebuffer_size);
		_palette.set_position({-.75f, -1.f});
		_palette.set_size({1.5f, .1f});
		_palette.draw(delta_time, total_time);
	}
}
