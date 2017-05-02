#include "heightfieldrenderer.h"

#include "Data/timestep.h"

#include <GL/glew.h>
#include <vector>

namespace vis
{
	HeightfieldRenderer::HeightfieldRenderer(EnsembleManager& ensemble)
		: Renderer(),
		  _ensemble{ensemble}
	{
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		_buffers.resize(3);
		glGenBuffers(3, _buffers.data());

		const Timestep& step = ensemble.currentStep();
		const unsigned field = 2;

		// Grid (position)
		auto grid = Renderer::genGrid(step.xSize(), step.ySize());
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Variance (height)
		glBindBuffer(GL_ARRAY_BUFFER, _buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*step.scalarsPerField(),
					 step.scalarFieldStart(field), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Indices (element buffer)
		auto indices = Renderer::genGridIndices(step.xSize(), step.ySize());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffers[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);

		_textures.push_back(0);
		glGenTextures(1, &_textures.back());
		glBindTexture(GL_TEXTURE_2D, _textures.back());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(step.xSize()), static_cast<int>(step.ySize()), 0, GL_RED, GL_FLOAT, step.scalarFieldStart(field+6));

		auto vertex_shader = Renderer::loadShader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl",
												  GL_VERTEX_SHADER);
		auto fragment_shader = Renderer::loadShader("/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",
													GL_FRAGMENT_SHADER);


		_program = glCreateProgram();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);
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
		glDrawElements(GL_TRIANGLES, _ensemble.currentStep().scalarsPerField()*6, GL_UNSIGNED_INT, 0);
	}
}
