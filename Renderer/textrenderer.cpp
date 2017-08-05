#include "textrenderer.h"

#include <algorithm>
#include <numeric>
#include <GLFW/glfw3.h>

#include "logger.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace vis
{
	TextRenderer::TextRenderer(unsigned height, const std::string& font)
		: Renderer{}
	{
		// Load FT2
		FT_Library ft;
		if(FT_Init_FreeType(&ft))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Freetype library could not be initialized";
			throw std::runtime_error("Freetype failed to init");
		}

		// Load font
		FT_Face face;
		if(FT_New_Face(ft, font.c_str(), 0, &face))
		{
			Logger::instance() << Logger::Severity::ERROR
							   << "Freetype library could not load " << font << ".";
			throw std::runtime_error("Freetype failed to load font");
		}

		// Set size
		FT_Set_Pixel_Sizes(face, 0, height);

		// Gather char dimensions
		auto glyph = face->glyph;
		for(FT_ULong i = 32; i < 128; ++i)
		{
			if(FT_Load_Char(face, i, FT_LOAD_RENDER))
			{
				Logger::instance() << Logger::Severity::WARNING
								   << "Freetype could not load character " << static_cast<char>(i) << " of font " << font;
				continue;
			}

			_atlas_width += glyph->bitmap.width;
			_atlas_height = std::max(_atlas_height, static_cast<int>(glyph->bitmap.rows));
		}

		glActiveTexture(GL_TEXTURE0);
		_texture = gen_texture();
		glBindTexture(GL_TEXTURE_2D, _texture);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	// Change unpack alignment because glyphs aren't 4 byte aligned
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _atlas_width, _atlas_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int x_offset = 0;
		_glyphs.resize(128);
		for(FT_ULong i = 32; i < 128; ++i)
		{
			if(FT_Load_Char(face, i, FT_LOAD_RENDER))
				continue;

			glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, 0, static_cast<GLsizei>(glyph->bitmap.width), static_cast<GLsizei>(glyph->bitmap.rows), GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

			_glyphs[i].advance = glm::vec2(glyph->advance.x >> 6, glyph->advance.y >> 6);
			_glyphs[i].size = glm::vec2(glyph->bitmap.width, glyph->bitmap.rows);
			_glyphs[i].bearing = glm::vec2(glyph->bitmap_left, glyph->bitmap_top);
			_glyphs[i].atlas_offset = static_cast<float>(x_offset) / _atlas_width;

			x_offset += glyph->bitmap.width;
		}
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);	// Reset unpack alignment to default value


		_vao = gen_vao();
		glBindVertexArray(_vao);
		_vbo = gen_buffer();
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);

		// Shaders
		auto vertex_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/text_vs.glsl"},	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader({"/home/eike/Documents/Code/Visualisation/Shader/text_fs.glsl"},	//TODO:change location to relative
										   GL_FRAGMENT_SHADER);
		_program = gen_program();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program);

		glDetachShader(_program, vertex_shader);
		glDetachShader(_program, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glUseProgram(_program);

		_position_uniform = glGetUniformLocation(_program, "origin");
		_viewport_uniform = glGetUniformLocation(_program, "viewport");
	}

	void TextRenderer::set_lines(const std::vector<std::string>& lines)
	{
		if(_lines != lines)
		{
			_lines = lines;
			update();
		}
	}

	void TextRenderer::set_positions(const std::vector<glm::vec2>& positions)
	{
		_positions = positions;
//		_positions.resize(std::max(_positions.size(), _lines.size()));
	}

	void TextRenderer::set_viewport(const glm::ivec2& viewport)
	{
		_viewport = viewport;
	}

	glm::vec2 TextRenderer::total_relative_size() const
	{
		return glm::vec2(std::max_element(_line_sizes.begin(), _line_sizes.end(),
										  [](const auto& a, const auto& b) { return a.x < b.x; })->x,
						 std::max_element(_line_sizes.begin(), _line_sizes.end(),
										  [](const auto& a, const auto& b) { return a.y < b.y; })->y);
	}

	std::vector<glm::vec2> TextRenderer::relative_sizes() const
	{
		auto sizes = _line_sizes;
		std::transform(sizes.begin(), sizes.end(), sizes.begin(), [&](const auto& x) { return x / glm::vec2(_viewport); });
		return sizes;
	}

	void TextRenderer::update()
	{
		auto quads = std::vector<float>();
		_last_vertex_indices.clear();
		_last_vertex_indices.reserve(_lines.size());
		_line_sizes.clear();
		_line_sizes.reserve(_lines.size());
		for(const auto& line : _lines)
		{
			auto line_pos = glm::vec2{0.f};
			_line_sizes.push_back(glm::vec2{-std::numeric_limits<float>::infinity()});

			for(const char& c : line)
			{
				// Check if the atlas contains c
				if(c < 0 || static_cast<size_t>(c) >= _glyphs.size())
				{
					Logger::instance() << Logger::Severity::WARNING
									   << "Texture atlas does not contain " << c
									   << ". The character will be skipped.";
					continue;
				}
				const auto& glyph = _glyphs[static_cast<size_t>(c)];

				// Calculate quad dimensions
				auto pos = line_pos + glyph.bearing;
				auto size = glyph.size;

				quads.push_back(pos.x);          quads.push_back(pos.y);          quads.push_back(glyph.atlas_offset);                               quads.push_back(0.f);
				quads.push_back(pos.x + size.x); quads.push_back(pos.y);          quads.push_back(glyph.atlas_offset + glyph.size.x / _atlas_width); quads.push_back(0.f);
				quads.push_back(pos.x);          quads.push_back(pos.y - size.y); quads.push_back(glyph.atlas_offset);                               quads.push_back(glyph.size.y / _atlas_height);
				quads.push_back(pos.x + size.x); quads.push_back(pos.y);          quads.push_back(glyph.atlas_offset + glyph.size.x / _atlas_width); quads.push_back(0.f);
				quads.push_back(pos.x);          quads.push_back(pos.y - size.y); quads.push_back(glyph.atlas_offset);                               quads.push_back(glyph.size.y / _atlas_height);
				quads.push_back(pos.x + size.x); quads.push_back(pos.y - size.y); quads.push_back(glyph.atlas_offset + glyph.size.x / _atlas_width); quads.push_back(glyph.size.y / _atlas_height);

				// Advance cursor
				line_pos += _glyphs[static_cast<size_t>(c)].advance;

				_line_sizes.back().x = std::max(_line_sizes.back().x, std::abs(pos.x + size.x));
				_line_sizes.back().y = std::max(_line_sizes.back().y, std::abs(pos.y - size.y));
			}
			_last_vertex_indices.push_back(static_cast<GLint>(quads.size()) / 4);
		}

		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(quads.size() * sizeof(float)), quads.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
	}

	void TextRenderer::draw(float /*delta_time*/, float /*total_time*/)
	{
		auto depthtest = glIsEnabled(GL_DEPTH_TEST);
		auto blending = glIsEnabled(GL_BLEND);
		int blendfunc;
		glGetIntegerv(GL_SRC_ALPHA, &blendfunc);

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(_program);
		glBindVertexArray(_vao);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(_program, "tex"), 0);
		glBindTexture(GL_TEXTURE_2D, _texture);

		glUniform2i(_viewport_uniform, _viewport.x, _viewport.y);
		GLint old_last = 0;
		for(size_t i = 0; i < _last_vertex_indices.size(); ++i)
		{
			glUniform2f(_position_uniform, _positions[i].x, _positions[i].y);
			glDrawArrays(GL_TRIANGLES, old_last, _last_vertex_indices[i]-old_last);
			old_last = _last_vertex_indices[i];
		}

		if(depthtest)
			glEnable(GL_DEPTH_TEST);
		if(!blending)
			glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, static_cast<GLenum>(blendfunc));
	}
}
