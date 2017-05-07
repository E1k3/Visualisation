#include "heightfieldrenderer.h"

#include "Data/timestep.h"

#include <GL/glew.h>
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

	void HeightfieldRenderer::setMVP(glm::mat4 mvp)
	{
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, glm::value_ptr(mvp));
	}

	void HeightfieldRenderer::draw()
	{
		glDrawElements(GL_TRIANGLES, static_cast<int>(_ensemble.currentStep().scalarsPerField()*6), GL_UNSIGNED_INT, 0);
	}
}
