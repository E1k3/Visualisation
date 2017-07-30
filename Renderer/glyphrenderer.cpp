#include "glyphrenderer.h"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"

namespace vis
{
	GlyphRenderer::GlyphRenderer(const std::vector<Field>& fields, InputManager& input)
		: Renderer{},
		  _input{input}
	{
		if(fields.size() <= 1)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Glyph renderer creation from less than 2 fields is impossible.";
			throw std::invalid_argument("Glyph renderer creation with < 2 fields");
		}
		if(fields.size() == 2)
			init_gaussian(fields);
		if(fields.size() >= 3)
			init_gmm(fields);
	}


	void GlyphRenderer::init_gaussian(const std::vector<Field>& fields)
	{
		constexpr unsigned mask_res_x = 1000;
		constexpr unsigned mask_res_y = 1000;

		auto mean_field = fields[0];
		auto var_field = fields[1];

		if(!mean_field.equal_layout(var_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean and variance fields have differing sizes.";
			throw std::runtime_error("Glyph rendering error");
			//TODO:ERROR handling. mean and var field have differing size.
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);

		// Grid (position)
		auto grid = gen_grid(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
		glEnableVertexAttribArray(0);
		_num_vertices = static_cast<int>(mean_field.area());


		// Mean (ring)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*mean_field.area()*mean_field.point_dimension(),
					 mean_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(1);

		// Variance (circle and background)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*var_field.area()*var_field.point_dimension(),
					 var_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, var_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(2);

		// Mask (glyph)
		auto mask = genMask(mask_res_x, mask_res_y);
		_texture = gen_texture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<int>(mask_res_x), static_cast<int>(mask_res_y),
					 0, GL_RGB, GL_FLOAT, mask.data());

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/glyph_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto geometry_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/glyph_gs.glsl"},	//TODO:change location to relative
										   GL_GEOMETRY_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/glyph_fs.glsl",
											"/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);

		_program = gen_program();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, geometry_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, geometry_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(geometry_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);

		glUniform1i(glGetUniformLocation(_program, "mask"), 0);
		glUniform2i(glGetUniformLocation(_program, "size"), mean_field.width(), mean_field.height());
		_mvp_uniform = glGetUniformLocation(_program, "mvp");
		_bounds_uniform = glGetUniformLocation(_program, "bounds");
		_bounds = glm::vec4(mean_field.minima()[0], mean_field.maxima()[0], var_field.minima()[0], var_field.maxima()[0]);
	}

	void GlyphRenderer::init_gmm(const std::vector<Field>& fields)
	{
		constexpr unsigned mask_res_x = 1000;
		constexpr unsigned mask_res_y = 1000;

		auto mean_field = fields[0];
		auto var_field = fields[1];
		auto weight_field = fields[2];

		if(!mean_field.equal_layout(var_field) || !mean_field.equal_layout(weight_field))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "The mean, variance and weight fields have differing sizes.";
			throw std::runtime_error("Glyph rendering error.");
			//TODO:ERROR handling. mean and var field have differing size.
		}
		if(mean_field.point_dimension() != 4)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Fields for GMM rendering do not have a point dimension of 4.";
			throw std::runtime_error("Glyph rendering error.");
			//TODO:ERROR handling. mean and var field have differing size.
		}

		_vao = gen_vao();
		glBindVertexArray(_vao);

		// Grid (position)
		auto grid = gen_grid(mean_field.width(), mean_field.height());
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float)*grid.size()),
					 &grid[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
		glEnableVertexAttribArray(0);
		_num_vertices = static_cast<int>(mean_field.area());

		// Mean (ring)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*mean_field.area()*mean_field.point_dimension(),
					 mean_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, mean_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(1);

		// Variance (circle and background)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*var_field.area()*var_field.point_dimension(),
					 var_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, var_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(2);

		// Weight (proportions)
		glBindBuffer(GL_ARRAY_BUFFER, gen_buffer());
		glBufferData(GL_ARRAY_BUFFER, static_cast<int>(sizeof(float))*weight_field.area()*weight_field.point_dimension(),
					 weight_field.data().data(), GL_STATIC_DRAW);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, weight_field.point_dimension()*static_cast<int>(sizeof(float)), 0);
		glEnableVertexAttribArray(3);

		// Mask (glyph)
		auto mask = genMask(mask_res_x, mask_res_y);
		_texture = gen_texture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<int>(mask_res_x), static_cast<int>(mask_res_y),
					 0, GL_RGB, GL_FLOAT, mask.data());

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto geometry_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_gs.glsl"},	//TODO:change location to relative
										 GL_GEOMETRY_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/gmm_glyph_fs_2.glsl",
											"/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);

		_program = gen_program();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, geometry_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, geometry_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(geometry_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);

		glUniform1i(glGetUniformLocation(_program, "mask"), 0);
		glUniform2i(glGetUniformLocation(_program, "size"), mean_field.width(), mean_field.height());
		_mvp_uniform = glGetUniformLocation(_program, "mvp");
		_bounds_uniform = glGetUniformLocation(_program, "bounds");
		_bounds = glm::vec4(mean_field.minimum(), mean_field.maximum(), var_field.minimum(), var_field.maximum());
	}

	void GlyphRenderer::draw(float delta_time, float total_time)
	{
		glBindVertexArray(_vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _texture);
		glUseProgram(_program);

		// Input handling
		using namespace glm;
		const float mousespeed = 0.001f;
		const float scrollspeed = -0.1f;

		auto trans_offset = _input.get_cursor_offset();
		trans_offset.x = -trans_offset.x;
		_translate += vec3(trans_offset * mousespeed * 1.f/_scale, 0.f);
		auto scale_offset = _input.get_scroll_offset_y();
		_scale *= 1.f + scale_offset*scrollspeed;

		auto framebuffer_size = _input.get_framebuffer_size();

		// MVP calculation
		auto model = scale(mat4{1.f}, vec3{192.f/96.f * framebuffer_size.y/framebuffer_size.x, 1.f, 1.f});
		auto view = glm::scale(glm::mat4{1.f}, vec3{_scale, _scale, 1.f}) * glm::translate(glm::mat4{1.f}, _translate);
		auto proj = mat4{1.f};
		auto mvp = proj * view * model;
		glUniformMatrix4fv(_mvp_uniform, 1, GL_FALSE, value_ptr(mvp));
		glUniform4f(_bounds_uniform, _bounds.x, _bounds.y, _bounds.z, _bounds.w);

		// Draw
		glDrawArrays(GL_POINTS, 0, _num_vertices);

		// Render palette
		_palette.set_bounds(_bounds);
		_palette.set_viewport(framebuffer_size);
		_palette.set_position({-.75f, -1.f});
		_palette.set_size({1.5f, .1f});
		_palette.draw(delta_time, total_time);
	}

	std::vector<float> GlyphRenderer::genMask(int width, int height) const
	{
		if(width < 0 || height < 0)
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Mask generation using negative dimensions.\n"
							   << "width: " << width << " height: " << height;
			throw std::invalid_argument("Negative mask generation dimensions");
		}

		auto mask = std::vector<float>(static_cast<size_t>(width * height) * 3);

		using namespace glm;
		const float r_outer = sqrt(2.f / (3.f * pi<float>()));
		const float r_inner = sqrt(r_outer * r_outer / 2.f);

		for(int y = 0; y < height; ++y)
		{
			for(int x = 0; x < width; ++x)
			{
				int i = (y*width + x) * 3;
				float dist = distance(vec2(x, y)/vec2(width, height), vec2(.5f, .5f));
				if(dist < r_inner)
				{
					mask[static_cast<size_t>(i)] = 1.f;
				}
				else if(dist < r_outer)
				{
					mask[static_cast<size_t>(i)+1] = 1.f;
				}
				else
				{
					mask[static_cast<size_t>(i)+2] = 1.f;
				}
			}
		}
		return mask;
	}
}
