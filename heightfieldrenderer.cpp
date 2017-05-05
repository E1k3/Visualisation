#include "heightfieldrenderer.h"

#include "Data/timestep.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

namespace vis
{
	HeightfieldRenderer::HeightfieldRenderer(EnsembleManager& ensemble)
		: Renderer(),
		  _ensemble{ensemble}
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
					 step.scalarFieldStart(field), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Indices (element buffer)
		auto indices = Renderer::genGridIndices(step.xSize(), step.ySize());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);

		// Average (color)
		glBindTexture(GL_TEXTURE_2D, genTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(step.xSize()), static_cast<int>(step.ySize()),
					 0, GL_RED, GL_FLOAT, step.scalarFieldStart(field));

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
		: HeightfieldRenderer(other._ensemble)
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

	void HeightfieldRenderer::draw()
	{
		using glm::vec3;
		using glm::mat4;
		auto model = glm::mat4{};
		auto view = glm::lookAt(vec3{1.8f}, vec3{0.f}, vec3{0.f, 0.f, 1.f});
		auto proj = glm::perspective(glm::radians(45.0f), 16.f / 9.f, 1.0f, 10.0f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, glm::value_ptr(mvp));
		glDrawElements(GL_TRIANGLES, static_cast<int>(_ensemble.currentStep().scalarsPerField()*6), GL_UNSIGNED_INT, 0);
	}
}
