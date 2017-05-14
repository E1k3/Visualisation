#include "glyphrenderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vis
{
	GlyphRenderer::GlyphRenderer(EnsembleManager* ensemble, InputManager* input)
		: Renderer{},
		  _ensemble{ensemble},
		  _input{input}
	{
		glBindVertexArray(genVao());

		const auto& step = ensemble->currentStep();
		const unsigned field = 2;

		// Grid (position)
		auto grid = genGrid(step.xSize()+1, step.ySize()+1);
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Average (color)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, genTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(step.xSize()), static_cast<int>(step.ySize()),
					 0, GL_RED, GL_FLOAT, step.scalarFieldStart(field));

		// Variance (height)
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, genTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, static_cast<int>(step.xSize()), static_cast<int>(step.ySize()),
					 0, GL_RED, GL_FLOAT, step.scalarFieldStart(field+6));

		// Indices (element buffer)
		auto indices = genGridIndices(step.xSize()+1, step.ySize()+1);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);

		// Shaders
		auto vertex_shader = loadShader("/home/eike/Documents/Code/Visualisation/Shader/glyph_vs.glsl",
										GL_VERTEX_SHADER);
		auto fragment_shader = loadShader("/home/eike/Documents/Code/Visualisation/Shader/glyph_fs.glsl",
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
		glUniform1i(glGetUniformLocation(prog, "avg"), 0);
		glUniform1i(glGetUniformLocation(prog, "var"), 1);
	}

	GlyphRenderer::GlyphRenderer(GlyphRenderer&& other) noexcept
		: GlyphRenderer{other._ensemble, other._input}
	{
		swap(*this, other);
	}

	GlyphRenderer& GlyphRenderer::operator=(GlyphRenderer other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	GlyphRenderer& GlyphRenderer::operator=(GlyphRenderer&& other) noexcept
	{
		swap(*this, other);
		return *this;
	}

	void GlyphRenderer::draw(float delta_time)
	{
		// Input handling
		using namespace glm;
		const float mousespeed = 0.002f;

		auto offset = _input->get_cursor_offset();
		offset.x = -offset.x;
		_cam_position += vec3(offset * mousespeed, 0.f);
		if(_input->get_key(GLFW_KEY_W))
			_cam_position += vec3(0.f, 0.f, -1.f) * delta_time;
		if(_input->get_key(GLFW_KEY_S))
			_cam_position += vec3(0.f, 0.f, 1.f) * delta_time;


		// MVP calculation
		auto model = scale(mat4{1.f}, vec3{192.f/96.f, 1.f, 1.f});
		auto view = lookAt(_cam_position, _cam_position + vec3{0.f, 0.01f, -1.f}, vec3{0.f, 0.f, 1.f});
		auto proj = perspective(radians(45.f), 16.f / 9.f, .05f, 10.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		unsigned x = _ensemble->currentStep().xSize() + 1;
		unsigned y = _ensemble->currentStep().ySize() + 1;
		glDrawElements(GL_TRIANGLES, static_cast<int>(x*y*6), GL_UNSIGNED_INT, 0);
	}
}
