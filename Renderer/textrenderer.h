#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <string>
#include <vector>
#include <tuple>
#include <glm/glm.hpp>

#include "renderer.h"

namespace vis
{
	class TextRenderer : public Renderer
	{
	public:
		explicit TextRenderer(unsigned height = 40, const std::string& font = "/usr/share/fonts/oiio/DroidSansMono.ttf");
		virtual ~TextRenderer() = default;

		void draw(float delta_time, float total_time) override;

		void set_text(const std::vector<std::tuple<std::string, glm::vec2>>& text);
		void set_viewport(const glm::ivec2& viewport);

		glm::ivec2 viewport() const;

	private:
		struct Glyph
		{
			glm::vec2 advance{0.f};
			glm::vec2 size{0.f};
			glm::vec2 bearing{0.f};
			float atlas_offset{0.f};
		};

		void update();

		int _atlas_width{0};
		int _atlas_height{0};

		GLuint _vao{0};
		GLuint _vbo{0};
		GLuint _texture{0};
		GLuint _program{0};

		glm::ivec2 _viewport{1};
		std::vector<std::tuple<std::string, glm::vec2>> _text{};

		int _num_vertices{0};
		std::vector<Glyph> _glyphs{};
	};
}

#endif // TEXTRENDERER_H
