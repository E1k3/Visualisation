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
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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

			glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, 0, static_cast<int>(glyph->bitmap.width), static_cast<int>(glyph->bitmap.rows), GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);

			_glyphs[i].advance = glm::vec2(glyph->advance.x >> 6, glyph->advance.y >> 6);
			_glyphs[i].size = glm::vec2(glyph->bitmap.width, glyph->bitmap.rows);
			_glyphs[i].bearing = glm::vec2(glyph->bitmap_left, glyph->bitmap_top);
			_glyphs[i].atlas_offset = static_cast<float>(x_offset) / _atlas_width;

			x_offset += glyph->bitmap.width;
		}

		//glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // TODO check if possible

		_vao = gen_vao();
		glBindVertexArray(_vao);
		_vbo = gen_buffer();
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);

		// Shaders
		auto vertex_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/text_vs.glsl",	//TODO:change location to relative
										 GL_VERTEX_SHADER);
		auto fragment_shader = load_shader("/home/eike/Documents/Code/Visualisation/Shader/text_fs.glsl",	//TODO:change location to relative
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
	}

	void TextRenderer::set_text(const std::vector<std::tuple<std::string, glm::vec2>>& text)
	{
		auto dirty = _text != text;
		_text = text;

		if(dirty)
			update();
	}

	void TextRenderer::set_viewport(const glm::ivec2& viewport)
	{
		auto dirty = _viewport != viewport;
		_viewport = viewport;

		if(dirty)
			update();
	}

	void TextRenderer::update()
	{
		auto quads = std::vector<float>();
		for(const auto& t : _text)
		{
			auto line = std::get<0>(t);
			auto line_pos = std::get<1>(t);
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
				auto pos = line_pos + glyph.bearing / glm::vec2(_viewport);
				auto size = glyph.size / glm::vec2(_viewport);

				quads.push_back(pos.x);          quads.push_back(pos.y);          quads.push_back(glyph.atlas_offset);                               quads.push_back(0.f);
				quads.push_back(pos.x + size.x); quads.push_back(pos.y);          quads.push_back(glyph.atlas_offset + glyph.size.x / _atlas_width); quads.push_back(0.f);
				quads.push_back(pos.x);          quads.push_back(pos.y - size.y); quads.push_back(glyph.atlas_offset);                               quads.push_back(glyph.size.y / _atlas_height);
				quads.push_back(pos.x + size.x); quads.push_back(pos.y);          quads.push_back(glyph.atlas_offset + glyph.size.x / _atlas_width); quads.push_back(0.f);
				quads.push_back(pos.x);          quads.push_back(pos.y - size.y); quads.push_back(glyph.atlas_offset);                               quads.push_back(glyph.size.y / _atlas_height);
				quads.push_back(pos.x + size.x); quads.push_back(pos.y - size.y); quads.push_back(glyph.atlas_offset + glyph.size.x / _atlas_width); quads.push_back(glyph.size.y / _atlas_height);

				// Advance cursor
				line_pos += _glyphs[static_cast<size_t>(c)].advance / glm::vec2(_viewport);
			}
		}

		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, static_cast<long>(quads.size() * sizeof(float)), quads.data(), GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);
		_num_vertices = static_cast<int>(quads.size()/4);
	}

	void TextRenderer::draw(float /*delta_time*/, float /*total_time*/)
	{
		auto blending = glIsEnabled(GL_BLEND);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(_program);
		glBindVertexArray(_vao);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(_program, "tex"), 0);
		glBindTexture(GL_TEXTURE_2D, _texture);

		glDrawArrays(GL_TRIANGLES, 0, _num_vertices);

		if(!blending)
			glDisable(GL_BLEND);
	}

	glm::ivec2 TextRenderer::viewport() const
	{
		return _viewport;
	}
}
