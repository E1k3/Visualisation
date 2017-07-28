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
		  _input{input}
	{
		if(fields.size() < 2)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Heightfield renderer creation with < 2 fields not possible.";
			throw std::invalid_argument("Heightfield renderer created using < 2 fields");
		}
		else if(fields.size() == 2)
			init_gaussian(fields);
		else
			init_gmm(fields);
	}

	void HeightfieldRenderer::init_gaussian(const std::vector<Field>& fields)
	{
		auto mean_field = fields[0];
		auto var_field = fields[1];

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
		auto grid = gen_grid_indexed(mean_field.width(), mean_field.height());
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
		auto vertex_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl",	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",	//TODO:change location to relative
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
		glUniform4f(_bounds_uniform, mean_field.minima()[0], mean_field.maxima()[0], var_field.minima()[0], var_field.maxima()[0]); // TODO:Save bounds as renderer state to scale data live.
	}

	void HeightfieldRenderer::init_gmm(const std::vector<Field>& fields)
	{
		auto mean_field = fields[0];
		auto var_field = fields[1];
		auto weight_field = fields[2];

		if(!mean_field.equal_layout(var_field) || !var_field.equal_layout(weight_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean, variance and weight fields have differing layouts.";
			throw std::invalid_argument("Heightfield creation using different fields");
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);

		// Grid (position)
		auto grid = gen_grid_indexed(mean_field.width(), mean_field.height());
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
		auto vertex_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_vs.glsl",	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_fs.glsl",	//TODO:change location to relative
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

	void HeightfieldRenderer::draw(float delta_time, float total_time)
	{
		glBindVertexArray(_vao);
		glUseProgram(_program);

		// Input handling
		using namespace glm;
		constexpr float mousespeed = 0.001f;
		constexpr float scrollspeed = 0.1f;
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
		auto model = scale(mat4{1.f}, vec3{192.f/96.f, 1.f, 1.f});
		auto view = lookAt(_cam_position, vec3(0.f), vec3{0.f, 0.f, 1.f});
		auto proj = perspective(radians(45.f), static_cast<float>(framebuffer_size.x)/framebuffer_size.y, .2f, 20.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		glUniform4f(_bounds_uniform, _bounds.x, _bounds.y, _bounds.z, _bounds.w);

		// Set time uniform
		if(_time_uniform != -1)
			glUniform1f(_time_uniform, total_time);

		// Draw
		glDrawElements(GL_TRIANGLES, _num_vertices, GL_UNSIGNED_INT, 0);

		_text.set_viewport(framebuffer_size);
		_text.set_text({{"0,22222", glm::vec2(-1.f, -.95f)}});
		_text.draw(delta_time, total_time);
	}
}
