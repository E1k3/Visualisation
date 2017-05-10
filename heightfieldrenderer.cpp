#include "heightfieldrenderer.h"

#include "Data/timestep.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <vector>

namespace vis
{
	HeightfieldRenderer::HeightfieldRenderer(EnsembleManager& ensemble, InputManager& input)
		: Renderer(),
		  _ensemble{ensemble},
		  _input{input}
	{
		glBindVertexArray(genVao());

		const Timestep& step = ensemble.currentStep();
		const unsigned field = 2;

		// Grid (position)
		auto grid = Renderer::genGrid(step.xSize(), step.ySize());
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Variance (height)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*step.scalarsPerField(),
					 step.scalarFieldStart(field+6), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Average (color)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*step.scalarsPerField(),
					 step.scalarFieldStart(field), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// Indices (element buffer)
		auto indices = Renderer::genGridIndices(step.xSize(), step.ySize());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);

		// Shaders
		auto vertex_shader = Renderer::loadShader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl",
												  GL_VERTEX_SHADER);
		auto fragment_shader = Renderer::loadShader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",
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
	}

	HeightfieldRenderer::HeightfieldRenderer(HeightfieldRenderer&& other) noexcept
		: HeightfieldRenderer(other._ensemble, other._input)
	{
		swap(*this, other);
	}

	HeightfieldRenderer& HeightfieldRenderer::operator=(HeightfieldRenderer other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	HeightfieldRenderer& HeightfieldRenderer::operator=(HeightfieldRenderer&& other) noexcept
	{
		swap(*this, other);
		return *this;
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

		// MVP calculation
		auto model = rotate(mat4{1.f}, 0.f, vec3{0.f, 0.f, 1.f});
		auto view = lookAt(_cam_position, _cam_position+_cam_direction, vec3{0.f, 0.f, 1.f});
		auto proj = perspective(radians(45.f), 16.f / 9.f, .2f, 10.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		// Draw
		glDrawElements(GL_TRIANGLES, static_cast<int>(_ensemble.currentStep().scalarsPerField()*6), GL_UNSIGNED_INT, 0);
	}
}
