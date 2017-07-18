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

		glBindVertexArray(gen_vao());

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
		_num_vertices = mean_field.area()*6;

		// Shaders
		auto vertex_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl",	//TODO:change location to relative
										GL_VERTEX_SHADER);
		auto fragment_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",	//TODO:change location to relative
										  GL_FRAGMENT_SHADER);
		auto prog = gen_program();
		glAttachShader(prog, vertex_shader);
		glAttachShader(prog, fragment_shader);
		glLinkProgram(prog);

		glDetachShader(prog, vertex_shader);
		glDetachShader(prog, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(prog);
		_mvp_uniform = glGetUniformLocation(prog, "mvp");
		_bounds_uniform = glGetUniformLocation(prog, "bounds");
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

		glBindVertexArray(gen_vao());

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
		_num_vertices = mean_field.area()*6;

		// Shaders
		auto vertex_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_vs.glsl",	//TODO:change location to relative
										GL_VERTEX_SHADER);
		auto fragment_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/gmm_heightfield_fs.glsl",	//TODO:change location to relative
										  GL_FRAGMENT_SHADER);
		auto prog = gen_program();
		glAttachShader(prog, vertex_shader);
		glAttachShader(prog, fragment_shader);
		glLinkProgram(prog);

		glDetachShader(prog, vertex_shader);
		glDetachShader(prog, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(prog);
		_mvp_uniform = glGetUniformLocation(prog, "mvp");
		_bounds_uniform = glGetUniformLocation(prog, "bounds");
		_time_uniform = glGetUniformLocation(prog, "time");
		glUniform4f(_bounds_uniform, mean_field.minimum(), mean_field.maximum(), var_field.minimum(), var_field.maximum()); // TODO:Save bounds as renderer state to scale data live.
	}

	void HeightfieldRenderer::draw(float /*delta_time*/, float total_time)
	{
		// Input handling
		using namespace glm;
		constexpr float mousespeed = 0.005f;
		constexpr float scrollspeed = 0.1f;

		_cam_position = rotateZ(_cam_position, _input.get_cursor_offset_x()*mousespeed);
		_cam_position = rotate(_cam_position,  _input.get_cursor_offset_y()*mousespeed, cross(-_cam_position, vec3(0.f, 0.f, 1.f)));
		_cam_position = normalize(_cam_position) * (length(_cam_position)*(1 + _input.get_scroll_offset_y() * scrollspeed));

		auto viewport = _input.get_framebuffer_size();

		// MVP calculation
		auto model = scale(mat4{1.f}, vec3{192.f/96.f * viewport.y/viewport.x, 1.f, 1.f});
		auto view = lookAt(_cam_position, vec3(0.f), vec3{0.f, 0.f, 1.f});
		auto proj = perspective(radians(45.f), 16.f / 9.f, .2f, 10.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));
		// Set time uniform
		if(_time_uniform != -1)
			glUniform1f(_time_uniform, total_time);

		// Draw
		glDrawElements(GL_TRIANGLES, _num_vertices, GL_UNSIGNED_INT, 0);
	}
}
