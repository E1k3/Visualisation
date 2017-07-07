#include "heightfieldrenderer.h"

#include "Data/timestep.h"
#include "logger.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <vector>

namespace vis
{
	HeightfieldRenderer::HeightfieldRenderer(const Timestep::ScalarField& mean_field, const Timestep::ScalarField& var_field, InputManager& input)
		: Renderer{},
		  _input{input}
	{
		if(!mean_field.same_dimensions(var_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean and variance fields have differing sizes.";
			throw std::runtime_error("Heightfield rendering error.");
			//TODO:ERROR handling. mean and var field have differing size.
		}

		glBindVertexArray(genVao());

		// Grid (position)
		auto grid = genGrid(mean_field._width, mean_field._height);
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
		glEnableVertexAttribArray(0);

		// Mean (color)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*mean_field.area(),
					 mean_field._data.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Variance (height)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*var_field.area(),
					 var_field._data.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// Indices (element buffer)
		auto indices = genGridIndices(mean_field._width, mean_field._height);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);
		_num_vertices = mean_field.area()*6;

		// Shaders
		auto vertex_shader = loadShader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl",
										GL_VERTEX_SHADER);
		auto fragment_shader = loadShader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",
										  GL_FRAGMENT_SHADER);
		auto prog = genProgram();
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
		glUniform4f(_bounds_uniform, mean_field.minimum(), mean_field.maximum(), var_field.minimum(), var_field.maximum()); // TODO:Save bounds as renderer state to scale data live.
	}

	void HeightfieldRenderer::draw(float delta_time)
	{
		// Input handling
		using namespace glm;
		const float mousespeed = 0.02f;
		_cam_direction = rotateZ(_cam_direction, radians(_input.get_cursor_offset_x() * mousespeed));

		auto y_offset = _input.get_cursor_offset_y();
		if((_cam_direction.z > -0.99f || y_offset > 0.f) && (_cam_direction.z < 0.99f || y_offset < 0.f))
			_cam_direction = rotate(_cam_direction, radians(y_offset * mousespeed), cross(_cam_direction, vec3{0.f, 0.f, 1.f}));
		_cam_direction = normalize(_cam_direction);

		if(_input.get_key(GLFW_KEY_W))
			_cam_position += _cam_direction * delta_time;
		if(_input.get_key(GLFW_KEY_A))
			_cam_position += normalize(cross(_cam_direction, vec3{0.f, 0.f, 1.f})) * -delta_time;
		if(_input.get_key(GLFW_KEY_S))
			_cam_position += _cam_direction * -delta_time;
		if(_input.get_key(GLFW_KEY_D))
			_cam_position += normalize(cross(_cam_direction, vec3{0.f, 0.f, 1.f})) * delta_time;

		auto viewport = _input.get_framebuffer_size();

		// MVP calculation
		auto model = scale(mat4{1.f}, vec3{192.f/96.f * viewport.y/viewport.x, 1.f, 1.f});
		auto view = lookAt(_cam_position, _cam_position+_cam_direction, vec3{0.f, 0.f, 1.f});
		auto proj = perspective(radians(45.f), 16.f / 9.f, .2f, 10.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		// Draw
		glDrawElements(GL_TRIANGLES, static_cast<int>(_num_vertices), GL_UNSIGNED_INT, 0);
	}
}
