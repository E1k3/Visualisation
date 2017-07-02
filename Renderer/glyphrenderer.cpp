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

		constexpr unsigned field = 6;
		const auto& mean_field = ensemble->currentStep().fields().at(field);
		const auto& var_field = ensemble->currentStep().fields().at(field+1);
		if(!mean_field.same_dimensions(var_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean and variance fields have differing sizes." << std::endl;
			throw std::runtime_error("Heightfield rendering error.");
			//TODO:ERROR handling. mean and var field have differing size.
		}

		// Grid (position)
		auto grid = genGrid(mean_field._width, mean_field._height);
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);


		// Mean (ring)
		glBindBuffer(GL_ARRAY_BUFFER, genBuffer());
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*mean_field.area(),
					 mean_field._data.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Variance (circle and background)
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
		auto size = glm::uvec2{mean_field._width-1, mean_field._height-1};
		glUniform2uiv(glGetUniformLocation(prog, "size"), 1, glm::value_ptr(size));
		_bounds_uniform = glGetUniformLocation(prog, "bounds");
		glUniform4f(_bounds_uniform, mean_field.minimum(), mean_field.maximum(), var_field.minimum(), var_field.maximum()); // TODO:Save bounds as renderer state to scale data live.
	}

	void GlyphRenderer::draw(float /*delta_time*/)
	{
		// Input handling
		using namespace glm;
		const float mousespeed = 0.001f;
		const float scrollspeed = -0.1f;

		auto trans_offset = _input->get_cursor_offset();
		trans_offset.x = -trans_offset.x;
		_translate += vec3(trans_offset * mousespeed * 1.f/_scale, 0.f);
		auto scale_offset = _input->get_scroll_offset_y();
		_scale *= 1.f + scale_offset*scrollspeed;

		auto viewport = _input->get_framebuffer_size();

		// MVP calculation
		auto model = scale(mat4{1.f}, vec3{192.f/96.f * viewport.y/viewport.x, 1.f, 1.f});
		auto view = glm::scale(glm::mat4{1.f}, vec3{_scale, _scale, 1.f}) * glm::translate(glm::mat4{1.f}, _translate);
		auto proj = mat4{1.f};
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));

		// Draw
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
