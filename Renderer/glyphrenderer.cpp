#include "glyphrenderer.h"

#include "logger.h"

#include <iostream>

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

		const unsigned field = 2;
		const auto& avg_field = ensemble->currentStep().fields().at(field);
		const auto& var_field = ensemble->currentStep().fields().at(field+6);
		if(!avg_field.same_dimensions(var_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The average and variance fields have differing sizes." << std::endl;
			throw std::runtime_error("Heightfield rendering error.");
			//TODO:ERROR handling. avg and var field have differing size.
		}

		// Grid (position)
		auto grid = genGrid(avg_field._width, avg_field._height);
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);


		// Average (ring)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*avg_field.num_scalars(),
					 avg_field._data.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Variance (circle and background)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*var_field.num_scalars(),
					 var_field._data.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// Indices (element buffer)
		auto indices = genGridIndices(avg_field._width, avg_field._height);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long>(sizeof(unsigned)*indices.size()),
					 &indices[0], GL_STATIC_DRAW);
		_num_vertices = avg_field.num_scalars()*6;

		// Mask (glyph)
		const unsigned width = 1000;
		const unsigned height = 1000;
		auto mask = genMask(width, height);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, genTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<int>(width), static_cast<int>(height),
					 0, GL_RGB, GL_FLOAT, mask.data());

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
		glUniform1i(glGetUniformLocation(prog, "mask"), 0);
		auto size = glm::uvec2{avg_field._width-1, avg_field._height-1};
		glUniform2uiv(glGetUniformLocation(prog, "size"), 1, glm::value_ptr(size));
		_bounds_uniform = glGetUniformLocation(prog, "bounds");
		glUniform4f(_bounds_uniform, avg_field._minimum, avg_field._maximum, var_field._minimum, var_field._maximum); // TODO:Save bounds as renderer state to scale data live.
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
		auto proj = perspective(radians(45.f), 16.f / 9.f, .05f, 20.f);
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		glDrawElements(GL_TRIANGLES, static_cast<int>(_num_vertices), GL_UNSIGNED_INT, 0);
	}

	std::vector<float> GlyphRenderer::genMask(unsigned width, unsigned height) const
	{
		auto mask = std::vector<float>(width * height * 3);

		using namespace glm;
		const float r_outer = sqrt(2.f / (3.f * pi<float>()));
		const float r_inner = sqrt(r_outer * r_outer / 2.f);

		for(unsigned y = 0; y < height; ++y)
		{
			for(unsigned x = 0; x < width; ++x)
			{
				unsigned i = (y*width + x) * 3;
				float dist = distance(vec2(x, y)/vec2(width, height), vec2(.5f, .5f));
				if(dist < r_inner)
				{
					mask[i] = 1.f;
				}
				else if(dist < r_outer)
				{
					mask[i+1] = 1.f;
				}
				else
				{
					mask[i+2] = 1.f;
				}
			}
		}
		return mask;
	}
}
